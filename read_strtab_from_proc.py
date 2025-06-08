import os
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import StringTableSection
from rich import print as rprint
from rich.console import Console
from rich.panel import Panel
from rich.table import Table


def find_all_strtab_sections(elffile):
    """找到所有的字符串表节"""
    strtabs = []
    for section in elffile.iter_sections():
        if isinstance(section, StringTableSection):
            strtabs.append(section)
    return strtabs


def find_segment_containing_offset(elffile, offset):
    for segment in elffile.iter_segments():
        if segment["p_type"] == "PT_LOAD":
            start = segment["p_offset"]
            end = start + segment["p_filesz"]
            if start <= offset < end:
                return segment
    return None


def find_load_base_address(pid, binary_path):
    """从 /proc/pid/maps 中找到二进制文件的实际加载基地址"""
    try:
        # 获取提供路径的真实路径（解析符号链接）
        try:
            real_binary_path = os.path.realpath(binary_path)
        except:
            real_binary_path = binary_path

        with open(f"/proc/{pid}/maps") as mapsf:
            for line in mapsf:
                fields = line.strip().split()
                if len(fields) >= 6:
                    maps_path = fields[5]
                    # 检查直接匹配或者真实路径匹配
                    if (
                        maps_path == binary_path
                        or maps_path == real_binary_path
                        or os.path.basename(maps_path) == os.path.basename(binary_path)
                    ):
                        # 找到第一个映射，这通常是可执行段的基地址
                        addr_range = fields[0]
                        start_s, _ = addr_range.split("-")
                        rprint(f"[green]Found binary in maps:[/green] {maps_path}")
                        return int(start_s, 16)
    except Exception as e:
        rprint(f"[red]Error reading maps:[/red] {e}")
    return None


def read_mem(pid, addr, size):
    with open(f"/proc/{pid}/mem", "rb") as memf:
        memf.seek(addr)
        return memf.read(size)


def parse_cstrings(data):
    strings = []
    current = []
    for b in data:
        if b == 0:
            if current:
                strings.append(bytes(current).decode(errors="replace"))
                current = []
        else:
            current.append(b)
    return strings


def create_string_table_info(strtab, segment, load_base, relative_vaddr, actual_vaddr):
    """创建字符串表信息表格"""
    table = Table(title=f"String Table: {strtab.name}")
    table.add_column("Property", style="cyan")
    table.add_column("Value", style="magenta")

    table.add_row("Section Name", strtab.name)
    table.add_row("File Offset", f"0x{strtab['sh_offset']:x}")
    table.add_row("Size", f"{strtab['sh_size']} bytes")
    table.add_row("Segment p_vaddr", f"0x{segment['p_vaddr']:x}")
    table.add_row("Segment p_offset", f"0x{segment['p_offset']:x}")
    table.add_row("Load Base Address", f"0x{load_base:x}")
    table.add_row("Relative Virtual Address", f"0x{relative_vaddr:x}")
    table.add_row("Actual Virtual Address", f"0x{actual_vaddr:x}")

    return table


def create_strings_display(strings, section_name):
    """创建字符串显示面板"""
    if not strings:
        return Panel(
            "[yellow]No strings found[/yellow]", title=f"{section_name} - Strings"
        )

    # 限制显示的字符串数量，避免输出太长
    display_strings = strings[:50] if len(strings) > 50 else strings

    string_text = "\n".join(
        [f"[green]{i:3d}:[/green] {repr(s)}" for i, s in enumerate(display_strings)]
    )
    if len(strings) > 50:
        string_text += f"\n[yellow]... and {len(strings) - 50} more strings[/yellow]"

    return Panel(string_text, title=f"{section_name} - Strings ({len(strings)} total)")


def main():
    console = Console()

    if len(sys.argv) != 3:
        console.print(f"[red]Usage:[/red] {sys.argv[0]} <pid> <path_to_binary>")
        sys.exit(1)

    pid = sys.argv[1]
    binary_path = sys.argv[2]

    console.print(
        Panel.fit(
            f"[bold]Reading String Tables from Process {pid}[/bold]\n[dim]Binary: {binary_path}[/dim]"
        )
    )

    # 找到进程中二进制文件的实际加载基地址
    load_base = find_load_base_address(pid, binary_path)
    if load_base is None:
        console.print(
            f"[red]Could not find load base address for {binary_path} in process {pid}[/red]"
        )
        sys.exit(1)

    console.print(f"[green]Found load base address:[/green] 0x{load_base:x}")

    with open(binary_path, "rb") as f:
        elffile = ELFFile(f)
        strtabs = find_all_strtab_sections(elffile)
        if not strtabs:
            console.print("[red]No String Table sections found[/red]")
            sys.exit(1)

        console.print(f"[green]Found {len(strtabs)} String Table section(s)[/green]")

        for i, strtab in enumerate(strtabs):
            console.print(
                f"\n[bold blue]Processing String Table {i + 1}/{len(strtabs)}: {strtab.name}[/bold blue]"
            )

            strtab_offset = strtab["sh_offset"]
            strtab_size = strtab["sh_size"]

            segment = find_segment_containing_offset(elffile, strtab_offset)
            if not segment:
                console.print(
                    f"[yellow]Warning: No PT_LOAD segment contains {strtab.name}[/yellow]"
                )
                continue

            # 计算字符串表的相对虚拟地址（相对于段基地址）
            relative_vaddr = segment["p_vaddr"] + (strtab_offset - segment["p_offset"])

            # 计算实际的虚拟地址（加上进程的实际加载基地址）
            actual_vaddr = load_base + relative_vaddr

            # 显示信息表格
            info_table = create_string_table_info(
                strtab, segment, load_base, relative_vaddr, actual_vaddr
            )
            console.print(info_table)

            # 验证地址范围
            with open(f"/proc/{pid}/maps") as mapsf:
                found = False
                for line in mapsf:
                    fields = line.strip().split()
                    addr_range = fields[0]
                    start_s, end_s = addr_range.split("-")
                    start = int(start_s, 16)
                    end = int(end_s, 16)
                    if start <= actual_vaddr < end:
                        console.print(
                            f"[green]✓ Address found in process memory map:[/green] {line.strip()}"
                        )
                        found = True
                        break
                if not found:
                    console.print(
                        f"[yellow]⚠ Warning: {strtab.name} virtual address not found in process memory maps.[/yellow]"
                    )

            # 读取进程内存中的字符串表数据
            try:
                data = read_mem(pid, actual_vaddr, strtab_size)
                strings = parse_cstrings(data)

                # 显示字符串
                strings_panel = create_strings_display(strings, strtab.name)
                console.print(strings_panel)

            except Exception as e:
                console.print(
                    f"[red]Failed to read process memory for {strtab.name}: {e}[/red]"
                )
                continue


if __name__ == "__main__":
    main()
