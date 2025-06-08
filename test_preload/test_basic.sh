#!/bin/bash

# 基本LD_PRELOAD功能测试
set -e

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 测试程序路径（Bazel会将可执行文件放在runfiles中）
MAIN_PROGRAM="$SCRIPT_DIR/bazel-bin/test_preload/main"
PRELOAD_LIB="$SCRIPT_DIR/bazel-bin/test_preload/libpreload.so"

echo "=== Basic LD_PRELOAD Test ==="

# 检查文件是否存在
if [[ ! -f "$MAIN_PROGRAM" ]]; then
    echo "ERROR: Main program not found at $MAIN_PROGRAM"
    exit 1
fi

if [[ ! -f "$PRELOAD_LIB" ]]; then
    echo "ERROR: Preload library not found at $PRELOAD_LIB"
    exit 1
fi

echo "Running program without LD_PRELOAD..."
"$MAIN_PROGRAM" > /tmp/normal_output.txt 2>&1

echo "Running program with LD_PRELOAD..."
LD_PRELOAD="$PRELOAD_LIB" "$MAIN_PROGRAM" > /tmp/preload_output.txt 2>&1

# 检查预加载库是否产生了输出
if grep -q "\[PRELOAD\]" /tmp/preload_output.txt; then
    echo "✓ LD_PRELOAD working correctly - intercepted function calls detected"
else
    echo "✗ LD_PRELOAD not working - no intercepted calls detected"
    exit 1
fi

# 检查统计信息
if grep -q "malloc calls:" /tmp/preload_output.txt; then
    echo "✓ Function call statistics generated"
else
    echo "✗ Function call statistics not found"
    exit 1
fi

echo "Basic test passed!"

# 清理临时文件
# rm -f /tmp/normal_output.txt /tmp/preload_output.txt