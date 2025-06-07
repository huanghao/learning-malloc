import argparse

import matplotlib.pyplot as plt
from elftools.elf.elffile import ELFFile


def print_segment_section_mapping(filename, show_orphans=True):
    with open(filename, "rb") as f:
        elf = ELFFile(f)

        # 读取所有节的信息
        sections = []
        for sec in elf.iter_sections():
            sections.append(
                {
                    "name": sec.name,
                    "addr": sec["sh_addr"],
                    "offset": sec["sh_offset"],
                    "size": sec["sh_size"],
                }
            )

        print(f"File: {filename}\n")

        segments = []
        sections_in_segments = set()  # 跟踪哪些节已经被包含在段中

        for i, segment in enumerate(elf.iter_segments()):
            seg_type = segment["p_type"]
            seg_offset = segment["p_offset"]
            seg_vaddr = segment["p_vaddr"]
            seg_filesz = segment["p_filesz"]
            seg_memsz = segment["p_memsz"]

            print(
                f"Segment {i}: Type={seg_type}, Offset=0x{seg_offset:x}, VirtAddr=0x{seg_vaddr:x}, FileSize=0x{seg_filesz:x} ({seg_filesz}), MemSize=0x{seg_memsz:x} ({seg_memsz})"
            )

            # 找出落在该段范围内的节
            contained_sections = []
            seg_start = seg_offset
            seg_end = seg_offset + seg_filesz

            for j, sec in enumerate(sections):
                sec_start = sec["offset"]
                sec_end = sec_start + sec["size"]
                # 判断节是否在段范围内
                if sec["size"] > 0 and sec_start >= seg_start and sec_end <= seg_end:
                    contained_sections.append(
                        f"{sec['name']}\t 0x{sec_start:x} - 0x{sec_end:x} 0x{sec['size']:x} ({sec['size']})"
                    )
                    sections_in_segments.add(j)  # 记录已包含的节

            if contained_sections:
                print("  Contains sections:")
                for name in contained_sections:
                    print(f"    {name}")
            else:
                print("  Contains no sections.")

            print()
            segments.append(
                {
                    "index": i,
                    "type": seg_type,
                    "start": seg_start,
                    "end": seg_end,
                    "memsz": seg_memsz,
                    "sections": contained_sections,
                }
            )

        # 显示不属于任何段的节
        orphan_sections = []
        for j, sec in enumerate(sections):
            if j not in sections_in_segments and sec["size"] > 0:
                orphan_sections.append(
                    f"{sec['name']}\t 0x{sec['offset']:x} - 0x{sec['offset'] + sec['size']:x} 0x{sec['size']:x} ({sec['size']})"
                )

        if orphan_sections and show_orphans:
            print("Sections not contained in any segment:")
            for section_info in orphan_sections:
                print(f"  {section_info}")
            print()

    return segments, sections, sections_in_segments


def plot_segments_sections(segments, sections, sections_in_segments, show_orphans=True):
    """
    用 matplotlib 画出 ELF 文件中段和节的文件内偏移范围
    横轴：文件偏移（bytes）
    纵轴：段或节编号（段用大号，节用小号，区分颜色）
    使用断轴来压缩空白区域，并在下方显示详细表格
    """

    # 找出所有有效的偏移范围
    all_ranges = []
    for seg in segments:
        if seg["end"] > seg["start"]:
            all_ranges.append((seg["start"], seg["end"]))

    # 只在显示orphan sections时才包含它们的范围
    for idx, sec in enumerate(sections):
        if sec["size"] > 0:
            # 如果不显示orphan sections，则跳过不在segments中的sections
            if not show_orphans and idx not in sections_in_segments:
                continue
            all_ranges.append((sec["offset"], sec["offset"] + sec["size"]))

    if not all_ranges:
        return

    # 按起始位置排序
    all_ranges.sort()

    # 合并重叠的范围并找出空白区域
    merged_ranges = []
    current_start, current_end = all_ranges[0]

    for start, end in all_ranges[1:]:
        if start <= current_end + 1000:  # 允许小间隙（1KB以内）
            current_end = max(current_end, end)
        else:
            merged_ranges.append((current_start, current_end))
            current_start, current_end = start, end
    merged_ranges.append((current_start, current_end))

    # 如果只有一个连续区域，不需要断轴
    if len(merged_ranges) == 1:
        # 原始绘图逻辑
        _plot_single_axis_with_table(
            segments, sections, sections_in_segments, show_orphans
        )
    else:
        # 使用断轴绘图
        _plot_broken_axis_with_table(
            segments, sections, merged_ranges, sections_in_segments, show_orphans
        )


def _plot_single_axis_with_table(
    segments, sections, sections_in_segments, show_orphans=True
):
    """原始的单轴绘图"""
    fig = plt.figure(figsize=(16, 8))

    # 创建单个图表
    ax_plot = fig.add_subplot(1, 1, 1)
    _plot_chart_only(ax_plot, segments, sections, sections_in_segments, show_orphans)

    plt.suptitle("ELF File Segment and Section Layout", fontsize=14)
    try:
        plt.tight_layout()
    except:
        pass  # 忽略tight_layout可能产生的警告
    plt.show()


def _plot_broken_axis_with_table(
    segments, sections, merged_ranges, sections_in_segments, show_orphans=True
):
    """断轴绘图"""
    fig = plt.figure(figsize=(16, 8))

    # 计算断轴子图
    n_subplots = len(merged_ranges)
    range_sizes = [end - start for start, end in merged_ranges]
    total_size = sum(range_sizes)
    width_ratios = [size / total_size for size in range_sizes]

    # 创建断轴子图
    gs = fig.add_gridspec(1, n_subplots, width_ratios=width_ratios, wspace=0.05)

    axes = []
    for i, (start_range, end_range) in enumerate(merged_ranges):
        ax = fig.add_subplot(gs[0, i])
        axes.append(ax)

        _plot_range(
            ax,
            segments,
            sections,
            start_range,
            end_range,
            i == 0,
            i == n_subplots - 1,
            sections_in_segments,
            show_orphans,
        )
        ax.set_xlim(start_range, end_range)

        if i == 0:
            ax.set_ylabel("Segments/Sections")
        if i == n_subplots // 2:
            ax.set_xlabel("File Offset (bytes)")

        if i > 0:
            ax.spines["left"].set_visible(False)
            ax.tick_params(left=False, labelleft=False)
        if i < n_subplots - 1:
            ax.spines["right"].set_visible(False)

    # 添加断轴标记
    for i in range(n_subplots - 1):
        _add_break_marks(axes[i], axes[i + 1])

    if axes:
        axes[0].legend(loc="upper left")

    plt.suptitle("ELF File Segment and Section Layout", fontsize=14)
    try:
        plt.tight_layout()
    except:
        pass  # 忽略tight_layout可能产生的警告
    plt.show()


def _plot_chart_only(ax, segments, sections, sections_in_segments, show_orphans=True):
    """只绘制图表部分，不包含表格"""
    y_seg_base = 10
    y_sec_base = 0
    height_seg = 3
    height_sec = 1

    # 画段（Segments）
    for seg in segments:
        ax.barh(
            y=seg["index"] * height_seg + y_seg_base,
            width=seg["end"] - seg["start"],
            left=seg["start"],
            height=height_seg * 0.8,
            color="skyblue",
            edgecolor="blue",
            label="Segment" if seg["index"] == 0 else None,
        )
        ax.text(
            seg["start"],
            seg["index"] * height_seg + y_seg_base + height_seg * 0.4,
            f"Seg {seg['index']} ({seg['type']})",
            va="center",
            ha="right",
            fontsize=8,
        )

    # 画节（Sections）
    legend_added_in = False
    legend_added_out = False

    for idx, sec in enumerate(sections):
        if sec["size"] == 0:
            continue

        # 区分在segment中和不在segment中的section
        if idx in sections_in_segments:
            color = "orange"
            edgecolor = "darkred"
            label = "Section (in segment)" if not legend_added_in else None
            legend_added_in = True
        else:
            # 如果不显示orphan sections，则跳过
            if not show_orphans:
                continue
            color = "lightcoral"  # 稍微不同的颜色
            edgecolor = "red"
            label = "Section (orphan)" if not legend_added_out else None
            legend_added_out = True

        ax.barh(
            y=idx * height_sec + y_sec_base,
            width=sec["size"],
            left=sec["offset"],
            height=height_sec * 0.8,
            color=color,
            edgecolor=edgecolor,
            label=label,
        )
        ax.text(
            sec["offset"],
            idx * height_sec + y_sec_base + height_sec * 0.4,
            sec["name"],
            va="center",
            ha="right",
            fontsize=6,
        )

    ax.set_xlabel("File Offset (bytes)")
    ax.set_yticks([])
    ax.legend(loc="upper right")


def _plot_range(
    ax,
    segments,
    sections,
    start_range,
    end_range,
    show_legend,
    is_last,
    sections_in_segments,
    show_orphans=True,
):
    """在指定范围内绘制段和节"""
    y_seg_base = 10
    y_sec_base = 0
    height_seg = 3
    height_sec = 1

    legend_added_seg = False
    legend_added_sec_in = False
    legend_added_sec_out = False

    # 画段（Segments）
    for seg in segments:
        # 检查段是否与当前范围有重叠
        if seg["end"] > start_range and seg["start"] < end_range:
            # 计算在当前范围内的部分
            plot_start = max(seg["start"], start_range)
            plot_end = min(seg["end"], end_range)

            ax.barh(
                y=seg["index"] * height_seg + y_seg_base,
                width=plot_end - plot_start,
                left=plot_start,
                height=height_seg * 0.8,
                color="skyblue",
                edgecolor="blue",
                label="Segment" if show_legend and not legend_added_seg else None,
            )

            if show_legend and not legend_added_seg:
                legend_added_seg = True

            # 只在段的起始位置显示标签
            if seg["start"] >= start_range:
                ax.text(
                    seg["start"],
                    seg["index"] * height_seg + y_seg_base + height_seg * 0.4,
                    f"Seg {seg['index']} ({seg['type']})",
                    va="center",
                    ha="right",
                    fontsize=8,
                )

    # 画节（Sections）
    for idx, sec in enumerate(sections):
        if sec["size"] == 0:
            continue

        sec_start = sec["offset"]
        sec_end = sec_start + sec["size"]

        # 检查节是否与当前范围有重叠
        if sec_end > start_range and sec_start < end_range:
            # 计算在当前范围内的部分
            plot_start = max(sec_start, start_range)
            plot_end = min(sec_end, end_range)

            # 区分在segment中和不在segment中的section
            if idx in sections_in_segments:
                color = "orange"
                edgecolor = "darkred"
                label = (
                    "Section (in segment)"
                    if show_legend and not legend_added_sec_in
                    else None
                )
                if show_legend and not legend_added_sec_in:
                    legend_added_sec_in = True
            else:
                # 如果不显示orphan sections，则跳过
                if not show_orphans:
                    continue
                color = "lightcoral"  # 稍微不同的颜色
                edgecolor = "red"
                label = (
                    "Section (orphan)"
                    if show_legend and not legend_added_sec_out
                    else None
                )
                if show_legend and not legend_added_sec_out:
                    legend_added_sec_out = True

            ax.barh(
                y=idx * height_sec + y_sec_base,
                width=plot_end - plot_start,
                left=plot_start,
                height=height_sec * 0.8,
                color=color,
                edgecolor=edgecolor,
                label=label,
            )

            # 只在节的起始位置显示标签
            if sec_start >= start_range:
                ax.text(
                    sec_start,
                    idx * height_sec + y_sec_base + height_sec * 0.4,
                    sec["name"],
                    va="center",
                    ha="right",
                    fontsize=6,
                )

    ax.set_yticks([])


def _add_break_marks(ax1, ax2):
    """在两个相邻的子图之间添加断轴标记"""
    # 获取图形坐标
    trans_fig = ax1.transData + ax1.transAxes.inverted()

    # 在右侧边缘添加断轴标记
    d = 0.015  # 断轴标记的大小
    kwargs = dict(transform=ax1.transAxes, color="k", clip_on=False, linewidth=1)
    ax1.plot((1 - d, 1 + d), (-d, +d), **kwargs)
    ax1.plot((1 - d, 1 + d), (1 - d, 1 + d), **kwargs)

    # 在左侧边缘添加断轴标记
    kwargs.update(transform=ax2.transAxes)
    ax2.plot((-d, +d), (-d, +d), **kwargs)
    ax2.plot((-d, +d), (1 - d, 1 + d), **kwargs)


def main():

    parser = argparse.ArgumentParser(
        description="Visualize ELF file segments and sections"
    )
    parser.add_argument("elf_file", help="ELF file to analyze")
    parser.add_argument(
        "--show-orphans",
        action="store_true",
        help="Show sections not contained in any segment",
    )

    args = parser.parse_args()

    show_orphans = args.show_orphans
    segments, sections, sections_in_segments = print_segment_section_mapping(
        args.elf_file, show_orphans
    )
    plot_segments_sections(segments, sections, sections_in_segments, show_orphans)


if __name__ == "__main__":
    main()
