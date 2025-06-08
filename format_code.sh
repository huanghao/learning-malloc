#!/bin/bash

# 代码格式化和检查脚本
# 在提交前运行此脚本来格式化和检查Python代码

echo "正在运行代码格式化和检查..."

# 1. 使用 isort 排序导入
echo "1. 运行 isort (导入排序)..."
isort --profile black --line-length=88 *.py
if [ $? -ne 0 ]; then
    echo "❌ isort 发现问题"
    exit 1
fi
echo "✅ isort 完成"

# 2. 使用 black 格式化代码
echo "2. 运行 black (代码格式化)..."
black --line-length=88 *.py
if [ $? -ne 0 ]; then
    echo "❌ black 发现问题"
    exit 1
fi
echo "✅ black 完成"

# 3. 使用 ruff 检查和自动修复代码问题
echo "3. 运行 ruff (代码检查)..."
ruff check --fix *.py
if [ $? -ne 0 ]; then
    echo "❌ ruff 发现问题"
    exit 1
fi
echo "✅ ruff 检查完成"

# 4. 使用 ruff 格式化代码
echo "4. 运行 ruff format..."
ruff format *.py
if [ $? -ne 0 ]; then
    echo "❌ ruff format 发现问题"
    exit 1
fi
echo "✅ ruff format 完成"

echo "🎉 所有代码格式化和检查已完成！"
echo "现在可以安全地提交代码了。"

