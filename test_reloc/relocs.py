from elftools.elf.elffile import ELFFile
from elftools.elf.sections import Section
from elftools.elf.enums import ENUM_RELOC_TYPE_x64, ENUM_RELOC_TYPE_i386
import sys
from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich.text import Text
from rich import box

console = Console()

def get_relocation_type_name(reloc_type, machine):
    """获取重定位类型的名称"""
    try:
        if machine == 'EM_X86_64':
            return ENUM_RELOC_TYPE_x64.get(reloc_type, f'Unknown({reloc_type})')
        elif machine == 'EM_386':
            return ENUM_RELOC_TYPE_i386.get(reloc_type, f'Unknown({reloc_type})')
        else:
            return f'Type_{reloc_type}'
    except:
        return f'Type_{reloc_type}'

def is_relocation_section(section):
    """检查是否为重定位节"""
    sh_type = section['sh_type']
    return sh_type == 'SHT_REL' or sh_type == 'SHT_RELA'

def print_relocations(filename):
    try:
        with open(filename, 'rb') as f:
            elf = ELFFile(f)

            # 显示文件信息
            machine = elf['e_machine']
            console.print(Panel(
                f"[bold white]ELF Relocation Analysis: [green]{filename}[/green][/bold white]\n"
                f"[dim]Architecture: {elf.get_machine_arch()}[/dim]\n"
                f"[dim]Machine: {machine}[/dim]\n"
                f"[dim]Class: {elf.elfclass}[/dim]",
                title="[bold blue]File Information[/bold blue]",
                box=box.DOUBLE
            ))

            relocation_sections = []

            # 查找所有重定位节
            for section in elf.iter_sections():
                if is_relocation_section(section):
                    relocation_sections.append(section)

            if not relocation_sections:
                console.print("\n[yellow]No relocation sections found[/yellow]")
                return

            console.print(f"\n[bold blue]Found {len(relocation_sections)} Relocation Section(s)[/bold blue]")

            for i, section in enumerate(relocation_sections):
                if i > 0:
                    console.print()

                # 创建重定位表
                table = Table(
                    title=f"[bold cyan]Relocations in {section.name}[/bold cyan]",
                    box=box.ROUNDED,
                    show_header=True,
                    header_style="bold magenta"
                )

                table.add_column("Offset", style="blue", justify="right")
                table.add_column("Type", style="yellow", width=20)
                table.add_column("Symbol", style="green", width=30)
                table.add_column("Addend", style="cyan", justify="right")

                reloc_count = 0
                is_rela = section['sh_type'] == 'SHT_RELA'

                for relocation in section.iter_relocations():
                    offset = f"0x{relocation['r_offset']:08x}"
                    reloc_type = relocation['r_info_type']
                    type_name = get_relocation_type_name(reloc_type, machine)

                    # 获取符号信息
                    symbol_name = "None"
                    if relocation['r_info_sym'] != 0:
                        try:
                            # 获取符号表
                            symtab = elf.get_section(section['sh_link'])
                            if symtab:
                                symbol = symtab.get_symbol(relocation['r_info_sym'])
                                if symbol and symbol.name:
                                    symbol_name = symbol.name
                                else:
                                    symbol_name = f"Symbol_{relocation['r_info_sym']}"
                        except:
                            symbol_name = f"Symbol_{relocation['r_info_sym']}"

                    # 获取addend（如果存在）
                    addend = ""
                    if is_rela and 'r_addend' in relocation.entry:
                        addend_val = relocation['r_addend']
                        if addend_val != 0:
                            addend = f"{addend_val:+d}"

                    # 为不同类型的符号添加样式
                    symbol_style = "green"
                    if symbol_name == "None":
                        symbol_style = "dim"
                    elif symbol_name.startswith("_"):
                        symbol_style = "yellow"
                    elif symbol_name in ["main", "printf", "malloc", "free"]:
                        symbol_style = "bold green"

                    table.add_row(
                        offset,
                        type_name,
                        Text(symbol_name, style=symbol_style),
                        addend
                    )
                    reloc_count += 1

                # 添加统计信息
                stats_text = f"Total relocations: [bold]{reloc_count}[/bold]"
                if is_rela:
                    stats_text += " | Type: [bold]RELA[/bold] (with addend)"
                else:
                    stats_text += " | Type: [bold]REL[/bold] (without addend)"

                console.print(Panel(table, subtitle=stats_text))

    except FileNotFoundError:
        console.print(f"[bold red]Error: File '{filename}' not found[/bold red]")
        sys.exit(1)
    except Exception as e:
        console.print(f"[bold red]Error analyzing file: {e}[/bold red]")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        console.print("[bold red]Usage:[/bold red] python relocs.py <elf-file>")
        sys.exit(1)
    print_relocations(sys.argv[1])
