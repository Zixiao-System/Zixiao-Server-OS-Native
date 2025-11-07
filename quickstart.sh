#!/bin/bash
# 快速开始脚本 - 自动检测环境并构建/运行

set -e

echo "========================================"
echo "  Zixiao OS - Quick Start Script"
echo "========================================"
echo ""

# 检测系统
OS=$(uname -s)
ARCH=$(uname -m)

echo "检测到系统: $OS $ARCH"
echo ""

# 检查 QEMU
if ! command -v qemu-system-aarch64 &> /dev/null; then
    echo "错误: 未找到 QEMU"
    echo ""
    echo "请安装 QEMU:"
    if [ "$OS" == "Darwin" ]; then
        echo "  brew install qemu"
    elif [ "$OS" == "Linux" ]; then
        echo "  sudo apt install qemu-system-arm  # Ubuntu/Debian"
        echo "  sudo pacman -S qemu-system-aarch64  # Arch"
    fi
    echo ""
    exit 1
fi

echo "✓ 找到 QEMU"

# 检查交叉编译器
HAS_CROSS=0
if command -v aarch64-elf-gcc &> /dev/null || \
   command -v aarch64-linux-gnu-gcc &> /dev/null; then
    echo "✓ 找到 ARM64 交叉编译器"
    HAS_CROSS=1
fi

# 根据环境选择构建方式
if [ "$ARCH" == "arm64" ] || [ "$ARCH" == "aarch64" ]; then
    echo ""
    echo "在 ARM64 主机上，使用系统编译器构建 ARM64 内核..."
    echo ""

    # 使用本机 Makefile
    make -f Makefile.native clean
    make -f Makefile.native arm64

    echo ""
    echo "========================================"
    echo "  构建成功！"
    echo "========================================"
    echo ""
    echo "启动 Zixiao OS ARM64..."
    echo "提示: 按 Ctrl+A 然后按 X 退出 QEMU"
    echo ""
    sleep 2

    qemu-system-aarch64 -M virt -cpu cortex-a57 \
        -kernel build/zixiao-arm64.elf \
        -m 512M -nographic

elif [ $HAS_CROSS -eq 1 ]; then
    echo ""
    echo "使用交叉编译器构建..."
    echo ""

    make clean
    make arm64

    echo ""
    echo "========================================"
    echo "  构建成功！"
    echo "========================================"
    echo ""
    echo "启动 Zixiao OS ARM64..."
    echo ""
    sleep 2

    ./run-arm64.sh
else
    echo ""
    echo "未找到合适的编译器。"
    echo ""
    echo "请查看 INSTALL.md 了解如何安装交叉编译工具链。"
    echo ""
    echo "对于 macOS ARM64 用户，最简单的方法是:"
    echo "  brew install qemu"
    echo "  make -f Makefile.native run"
    exit 1
fi
