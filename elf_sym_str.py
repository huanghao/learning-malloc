import argparse
import sys

from elftools.elf.elffile import ELFFile
from elftools.elf.sections import StringTableSection, SymbolTableSection
from rich import box
from rich.console import Console
from rich.panel import Panel
from rich.table import Table
from rich.text import Text

console = Console()


def print_symbols(elffile, section):
    section_name = section.name

    # 创建符号表
    table = Table(
        title=f"[bold cyan]Symbols in {section_name}[/bold cyan]",
        box=box.ROUNDED,
        show_header=True,
        header_style="bold magenta",
    )

    table.add_column("Name", style="green", width=30)
    table.add_column("Address", style="blue", justify="right")
    table.add_column("Size", style="yellow", justify="right")
    table.add_column("Bind", style="red")
    table.add_column("Type", style="purple")
    table.add_column("Section", style="cyan", justify="right")

    symbol_count = 0
    for symbol in section.iter_symbols():
        name = symbol.name if symbol.name else "[no name]"
        addr = f"0x{symbol['st_value']:x}"
        size = str(symbol["st_size"])
        bind = symbol["st_info"].bind
        typ = symbol["st_info"].type
        shndx = str(symbol["st_shndx"])

        # 为特殊符号添加样式
        name_style = (
            "bold green"
            if symbol.name and not symbol.name.startswith(".")
            else "dim green"
        )
        if symbol["st_value"] == 0 and symbol["st_size"] == 0:
            name_style = "dim"

        table.add_row(Text(name, style=name_style), addr, size, bind, typ, shndx)
        symbol_count += 1

    # 添加统计信息
    stats_text = f"Total symbols: [bold]{symbol_count}[/bold]"

    console.print(Panel(table, subtitle=stats_text))


def print_strings(elffile, section):
    section_name = section.name

    # 获取字符串表的原始数据
    data = section.data()

    # 解析字符串表中的所有字符串
    strings = []
    current_string = b""
    offset = 0

    for byte in data:
        if byte == 0:  # 字符串结束符
            if current_string:  # 如果当前字符串不为空
                try:
                    decoded_string = current_string.decode("utf-8")
                    strings.append(
                        (offset - len(current_string), decoded_string, False)
                    )
                except UnicodeDecodeError:
                    # 如果无法解码为UTF-8，使用latin-1作为回退
                    decoded_string = current_string.decode("latin-1")
                    strings.append((offset - len(current_string), decoded_string, True))
                current_string = b""
        else:
            current_string += bytes([byte])
        offset += 1

    # 创建字符串表
    table = Table(
        title=f"[bold cyan]Strings in {section_name}[/bold cyan]",
        box=box.SIMPLE,
        show_header=True,
        header_style="bold magenta",
    )

    table.add_column("Offset", style="blue", justify="right", width=8)
    table.add_column("Length", style="yellow", justify="right", width=8)
    table.add_column("String", style="green")

    for offset, string, is_binary in strings:
        # 限制字符串显示长度，避免过长
        display_string = string
        if len(string) > 60:
            display_string = string[:57] + "..."

        # 为二进制数据添加特殊样式
        if is_binary:
            string_style = "dim red"
            display_string = f"[binary] {display_string}"
        else:
            string_style = "green"
            # 为特殊字符串添加样式
            if string.startswith("."):
                string_style = "cyan"
            elif string in ["main", "__libc_start_main", "_start"]:
                string_style = "bold green"
            elif string.startswith("__"):
                string_style = "yellow"

        table.add_row(
            str(offset), str(len(string)), Text(display_string, style=string_style)
        )

    # 添加统计信息
    stats_text = f"Total strings: [bold]{len(strings)}[/bold] | Section size: [bold]{len(data)}[/bold] bytes"

    console.print(Panel(table, subtitle=stats_text))


def main():
    parser = argparse.ArgumentParser(
        description="Analyze ELF file symbols and string tables",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Examples:
  python elf_sym_str.py program          # Show symbols only (default)
  python elf_sym_str.py program -s       # Show string tables only
  python elf_sym_str.py program -s -y    # Show both symbols and strings
        """,
    )

    parser.add_argument("filename", help="ELF file to analyze")
    parser.add_argument(
        "-s",
        "--strings",
        action="store_true",
        help="Show string tables instead of symbols",
    )
    parser.add_argument(
        "-y",
        "--symbols",
        action="store_true",
        help="Show symbols (use with -s to show both)",
    )

    args = parser.parse_args()

    # 确定要显示的内容
    show_symbols = True  # 默认显示符号表
    show_strings = False

    if args.strings:
        show_strings = True
        if not args.symbols:
            show_symbols = False  # 如果只指定了 -s，就只显示字符串表

    try:
        with open(args.filename, "rb") as f:
            elffile = ELFFile(f)

            # 显示文件信息
            console.print(
                Panel(
                    f"[bold white]ELF File Analysis: [green]{args.filename}[/green][/bold white]\n"
                    f"[dim]Architecture: {elffile.get_machine_arch()}[/dim]\n"
                    f"[dim]Class: {elffile.elfclass}[/dim]\n"
                    f"[dim]Data: {elffile['e_ident']['EI_DATA']}[/dim]",
                    title="[bold blue]File Information[/bold blue]",
                    box=box.DOUBLE,
                )
            )

            # 遍历所有节，找到所有符号表和字符串表
            symbol_tables = []
            string_tables = []

            for section in elffile.iter_sections():
                if isinstance(section, SymbolTableSection):
                    symbol_tables.append(section)
                elif isinstance(section, StringTableSection):
                    string_tables.append(section)

            # 根据参数决定显示内容
            if show_symbols:
                if symbol_tables:
                    console.print(
                        f"\n[bold blue]Found {len(symbol_tables)} Symbol Table(s)[/bold blue]"
                    )
                    for i, symtab in enumerate(symbol_tables):
                        if i > 0:
                            console.print()
                        print_symbols(elffile, symtab)
                else:
                    console.print("\n[yellow]No symbol tables found[/yellow]")

            if show_strings:
                if show_symbols and symbol_tables:
                    console.print()  # 在符号表和字符串表之间添加空行

                if string_tables:
                    console.print(
                        f"\n[bold blue]Found {len(string_tables)} String Table(s)[/bold blue]"
                    )
                    for i, strtab in enumerate(string_tables):
                        if i > 0:
                            console.print()
                        print_strings(elffile, strtab)
                else:
                    console.print("\n[yellow]No string tables found[/yellow]")

    except FileNotFoundError:
        console.print(f"[bold red]Error: File '{args.filename}' not found[/bold red]")
        sys.exit(1)
    except Exception as e:
        console.print(f"[bold red]Error analyzing file: {e}[/bold red]")
        sys.exit(1)


if __name__ == "__main__":
    main()
