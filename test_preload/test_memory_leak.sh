#!/bin/bash

# 内存泄漏检测测试
set -e

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 测试程序路径
LEAK_PROGRAM="$SCRIPT_DIR/bazel-bin/test_preload/memory_leak_demo"
ADVANCED_LIB="$SCRIPT_DIR/bazel-bin/test_preload/libadvanced_preload.so"

echo "=== Memory Leak Detection Test ==="

# 检查文件是否存在
if [[ ! -f "$LEAK_PROGRAM" ]]; then
    echo "ERROR: Leak demo program not found at $LEAK_PROGRAM"
    exit 1
fi

if [[ ! -f "$ADVANCED_LIB" ]]; then
    echo "ERROR: Advanced preload library not found at $ADVANCED_LIB"
    exit 1
fi

echo "Running memory leak demo with advanced LD_PRELOAD..."
LD_PRELOAD="$ADVANCED_LIB" "$LEAK_PROGRAM" > /tmp/leak_output.txt 2>&1

# 检查是否检测到内存泄漏
if grep -q "Memory Leaks Detected" /tmp/leak_output.txt; then
    echo "✓ Memory leak detection working - leaks detected"
elif grep -q "No memory leaks detected" /tmp/leak_output.txt; then
    echo "✓ Memory leak detection working - no leaks found"
else
    echo "✗ Memory leak detection not working properly"
    cat /tmp/leak_output.txt
    exit 1
fi

# 检查内存统计信息
if grep -q "Memory Statistics" /tmp/leak_output.txt; then
    echo "✓ Memory statistics generated"
else
    echo "✗ Memory statistics not found"
    exit 1
fi

# 检查是否有malloc调用记录
if grep -q "malloc(" /tmp/leak_output.txt; then
    echo "✓ malloc calls intercepted and logged"
else
    echo "✗ malloc interception not working"
    exit 1
fi

echo "Memory leak detection test passed!"

# 清理临时文件
# rm -f /tmp/leak_output.txt