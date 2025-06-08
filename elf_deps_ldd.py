import os
import re
import subprocess
import sys

# 记录已解析的库，避免重复
loaded = set()

# 存储节点和边的信息
nodes = set()
edges = []  # 保存 (src, dst, order) 的元组
edge_order = 0  # 全局计数器，跟踪遍历顺序


def parse_ldd_output(ldd_output):
    """解析ldd命令的输出，返回依赖库的路径列表"""
    dependencies = []

    for line in ldd_output.strip().split("\n"):
        line = line.strip()
        if not line:
            continue

        # 跳过linux-vdso.so.1这种虚拟库
        if "linux-vdso.so" in line:
            continue

        # 处理不同格式的ldd输出
        # 格式1: libname.so => /path/to/libname.so (0x...)
        # 格式2: /lib64/ld-linux-x86-64.so.2 (0x...)
        # 格式3: libname.so (0x...) [当库在当前目录或者是静态链接时]

        if "=>" in line:
            # 格式1: libname.so => /path/to/libname.so (0x...)
            parts = line.split("=>")
            if len(parts) >= 2:
                lib_path = parts[1].strip().split("(")[0].strip()
                if lib_path and lib_path != "(0x" and lib_path != "":
                    dependencies.append(lib_path)
        else:
            # 格式2和3: 直接的路径或库名
            # 移除地址信息 (0x...)
            lib_path = re.sub(r"\s*\(0x[0-9a-f]+\)\s*$", "", line).strip()
            if lib_path and not lib_path.startswith("("):
                # 如果是绝对路径，直接使用；否则可能需要查找
                if os.path.isfile(lib_path):
                    dependencies.append(lib_path)

    return dependencies


def get_library_dependencies(lib_path):
    """使用ldd命令获取库的依赖信息"""
    try:
        # 运行ldd命令
        result = subprocess.run(
            ["ldd", lib_path], capture_output=True, text=True, check=True
        )
        return parse_ldd_output(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"[Warning] Failed to run ldd on {lib_path}: {e}")
        return []
    except FileNotFoundError:
        print("[Warning] ldd command not found. Please install it.")
        return []


def get_library_name(lib_path):
    """从库路径中提取库名"""
    return os.path.basename(lib_path)


def build_dependency_graph(lib_path):
    """深度优先遍历构建依赖图"""
    global edge_order

    lib_name = get_library_name(lib_path)

    if lib_name in loaded:
        return
    loaded.add(lib_name)

    # 检查文件是否存在
    if not os.path.isfile(lib_path):
        print(f"[Warning] Library file {lib_path} not found.")
        nodes.add(lib_name)
        return

    nodes.add(lib_name)

    # 获取这个库的依赖
    deps = get_library_dependencies(lib_path)

    for dep_path in deps:
        dep_name = get_library_name(dep_path)
        edge_order += 1
        edges.append((lib_name, dep_name, edge_order))
        # 递归构建依赖图
        build_dependency_graph(dep_path)


def generate_dot_format(title):
    """生成DOT格式的文本"""
    dot_lines = ['digraph "Shared Library Dependencies (via ldd)" {']
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

    # 重置全局状态
    nodes = set()
    edges = []
    edge_order = 0
    loaded = set()

    # 检查输入文件是否存在
    if not os.path.isfile(elf_path):
        print(f"Error: File {elf_path} does not exist.")
        sys.exit(1)

    # 主程序作为根节点
    main_name = os.path.basename(elf_path)
    nodes.add(main_name)
    loaded.add(main_name)

    print(f"Analyzing dependencies for: {elf_path}")
    print("Using ldd to parse shared library dependencies...")

    # 获取主程序的直接依赖
    main_deps = get_library_dependencies(elf_path)

    # 为主程序的每个依赖创建边并递归分析
    for dep_path in main_deps:
        dep_name = get_library_name(dep_path)
        edge_order += 1
        edges.append((main_name, dep_name, edge_order))
        build_dependency_graph(dep_path)

    # 生成DOT格式文本
    title = f"Dependency Graph for {os.path.basename(elf_path)}"
    dot_content = generate_dot_format(title)

    # 生成输出文件名，带上输入文件的名字信息
    base_name = os.path.splitext(os.path.basename(elf_path))[0]
    output_file = f"dependency_graph_{base_name}_ldd.dot"
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
        print("Usage: python elf_deps_ldd.py <elf-file>")
        print(
            "\nThis script uses 'ldd' command to analyze shared library dependencies."
        )
        print("It performs depth-first traversal and maintains dependency order.")
        sys.exit(1)

    main(sys.argv[1])
