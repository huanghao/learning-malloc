#!/bin/bash
echo "1. 运行 ruff (代码检查)..."
ruff check --fix scripts/*.py
if [ $? -ne 0 ]; then
    echo "❌ ruff 发现问题"
    exit 1
fi

echo "2. 运行 isort (导入排序)..."
isort --profile black --line-length=88 scripts/*.py
if [ $? -ne 0 ]; then
    echo "❌ isort 发现问题"
    exit 1
fi

echo "3. 运行 black (代码格式化)..."
black --line-length=88 scripts/*.py
if [ $? -ne 0 ]; then
    echo "❌ black 发现问题"
    exit 1
fi

echo "🎉 所有代码格式化和检查已完成！"
echo "现在可以安全地提交代码了。"
