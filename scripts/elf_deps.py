import os
import sys

from elftools.elf.elffile import ELFFile

# 记录已解析的库，避免重复
loaded = set()

# 存储节点和边的信息
nodes = set()
edges = []  # 改为列表，保存 (src, dst, order) 的元组
edge_order = 0  # 全局计数器，跟踪遍历顺序

# 模拟系统中共享库文件的路径查找（简单版本）
# 你可以根据需要扩展搜索路径
SEARCH_PATHS = [
    ".",  # 当前目录
    "/lib",
    "/usr/lib",
    "/lib64",
    "/usr/lib64",
    # 更多路径可加
]


def find_library_path(libname):
    for path in SEARCH_PATHS:
        candidate = os.path.join(path, libname)
        if os.path.isfile(candidate):
            return candidate
    return None  # 找不到


def parse_needed_libraries(filepath):
    """读取ELF文件，返回依赖库名列表"""
    with open(filepath, "rb") as f:
        elf = ELFFile(f)
        dynamic = elf.get_section_by_name(".dynamic")
        if not dynamic:
            return []
        needed = []
        strtab = elf.get_section(dynamic["sh_link"])
        for tag in dynamic.iter_tags():
            if tag.entry.d_tag == "DT_NEEDED":
                libname = strtab.get_string(tag.entry.d_val)
                needed.append(libname)
        return needed


def build_dependency_graph(libname):
    global edge_order

    if libname in loaded:
        return
    loaded.add(libname)

    libpath = find_library_path(libname)
    if libpath is None:
        print(f"[Warning] Library {libname} not found in search paths.")
        # 仍然加入图节点
        nodes.add(libname)
        return

    nodes.add(libname)
    deps = parse_needed_libraries(libpath)
    for dep in deps:
        edge_order += 1
        edges.append((libname, dep, edge_order))
        build_dependency_graph(dep)


def generate_dot_format(title):
    """生成DOT格式的文本"""
    dot_lines = ['digraph "Shared Library Dependencies" {']
    dot_lines.append(f'    label="{title}";')  # 图的标题
    dot_lines.append("    labelloc=t;")  # 标题位置在顶部
    dot_lines.append("    fontsize=16;")  # 标题字体大小
    dot_lines.append('    fontname="Arial Bold";')  # 标题字体
    dot_lines.append("    rankdir=TB;")  # 从上到下布局
    dot_lines.append("    node [shape=box, style=rounded];")

    # 添加节点
    for node in sorted(nodes):
        dot_lines.append(f'    "{node}";')

    # 添加边，包含顺序信息
    for src, dst, order in edges:
        dot_lines.append(f'    "{src}" -> "{dst}" [label="{order}"];')

    dot_lines.append("}")
    return "\n".join(dot_lines)


def main(elf_path):
    global nodes, edges, edge_order, loaded
    nodes = set()
    edges = []
    edge_order = 0
    loaded = set()

    # 主程序用特殊名称，或者用文件名
    main_libname = os.path.basename(elf_path)
    nodes.add(main_libname)
    loaded.add(main_libname)

    print(f"Analyzing dependencies for: {elf_path}")
    print("Using ELF parsing to analyze shared library dependencies...")

    deps = parse_needed_libraries(elf_path)
    for dep in deps:
        edge_order += 1
        edges.append((main_libname, dep, edge_order))
        build_dependency_graph(dep)

    # 生成DOT格式文本
    title = f"Dependency Graph for {os.path.basename(elf_path)}"
    dot_content = generate_dot_format(title)

    # 生成输出文件名，带上输入文件的名字信息
    base_name = os.path.splitext(os.path.basename(elf_path))[0]
    output_file = f"dependency_graph_{base_name}_elf.dot"
    with open(output_file, "w") as f:
        f.write(dot_content)

    print(f"\nDependency graph saved to {output_file}")
    print(f"Found {len(nodes)} libraries and {len(edges)} dependencies")

    # 显示统计信息
    print("\nLibraries found:")
    for node in sorted(nodes):
        print(f"  - {node}")

    print("\nDOT content preview:")
    print(dot_content)

    # 提示如何可视化
    image_base = f"dependency_graph_{base_name}"
    print("\nTo visualize the graph, you can use:")
    print(f"  dot -Tpng {output_file} -o {image_base}.png")
    print(f"  dot -Tsvg {output_file} -o {image_base}.svg")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python elf_deps.py <elf-file>")
        sys.exit(1)
    main(sys.argv[1])
