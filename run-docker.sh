#!/bin/bash
# Docker 快速启动脚本 - 无需安装交叉编译工具链

echo "=================================="
echo "  Zixiao OS - Docker 构建方案"
echo "=================================="
echo ""

# 检查 Docker
if ! command -v docker &> /dev/null; then
    echo "错误: 未找到 Docker"
    echo ""
    echo "请安装 Docker Desktop for Mac:"
    echo "  https://www.docker.com/products/docker-desktop"
    exit 1
fi

echo "✓ Docker 已安装"
echo ""

# 创建 Docker 镜像
echo "创建构建环境..."
cat > Dockerfile.build << 'EOF'
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    qemu-system-arm \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
EOF

# 构建 Docker 镜像
if ! docker images | grep -q "zixiao-builder"; then
    echo "构建 Docker 镜像（仅首次需要，约 1-2 分钟）..."
    docker build -f Dockerfile.build -t zixiao-builder .
fi

echo ""
echo "✓ 构建环境就绪"
echo ""

# 创建临时 Makefile 用于 Ubuntu
cat > Makefile.docker << 'EOF'
ARM64_CC := aarch64-linux-gnu-gcc
ARM64_AS := aarch64-linux-gnu-as
ARM64_LD := aarch64-linux-gnu-ld

ARM64_CFLAGS := -ffreestanding -O2 -Wall -Wextra -nostdlib \
                -fno-builtin -fno-stack-protector -Isrc/include

ARM64_LDFLAGS := -nostdlib -T src/arch/arm64/linker.ld

ARM64_OBJS := build/arm64/boot.o \
              build/arm64/kernel_main.o \
              build/arm64/uart.o \
              build/arm64/exceptions.o \
              build/arm64/string.o \
              build/arm64/printf.o \
              build/arm64/vfs.o \
              build/arm64/initrd.o

.PHONY: all clean

all: build/zixiao-arm64.elf

build/arm64:
	mkdir -p build/arm64

build/arm64/%.o: src/arch/arm64/boot/%.S | build/arm64
	$(ARM64_AS) $< -o $@

build/arm64/boot.o: src/arch/arm64/boot/boot.S | build/arm64
	$(ARM64_AS) $< -o $@

build/arm64/kernel_main.o: src/arch/arm64/kernel_main.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/uart.o: src/arch/arm64/drivers/uart.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/exceptions.o: src/arch/arm64/interrupts/exceptions.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/string.o: src/kernel/lib/string.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/printf.o: src/kernel/lib/printf.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/vfs.o: src/kernel/fs/vfs.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/arm64/initrd.o: src/kernel/fs/initrd.c | build/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

build/zixiao-arm64.elf: $(ARM64_OBJS)
	$(ARM64_LD) $(ARM64_LDFLAGS) -o $@ $^

clean:
	rm -rf build
EOF

# 在 Docker 中构建
echo "在 Docker 容器中编译内核..."
docker run --rm -v "$(pwd):/workspace" zixiao-builder make -f Makefile.docker clean
docker run --rm -v "$(pwd):/workspace" zixiao-builder make -f Makefile.docker

if [ $? -eq 0 ]; then
    echo ""
    echo "=================================="
    echo "  ✅ 编译成功！"
    echo "=================================="
    echo ""
    echo "现在运行内核..."
    echo "提示: 按 Ctrl+A 然后按 X 退出 QEMU"
    echo ""
    sleep 2

    # 在 Docker 中运行 QEMU
    docker run --rm -it -v "$(pwd):/workspace" zixiao-builder \
        qemu-system-aarch64 -M virt -cpu cortex-a57 \
        -kernel build/zixiao-arm64.elf -m 512M -nographic
else
    echo ""
    echo "编译失败，请检查错误信息"
    exit 1
fi

# 清理临时文件
rm -f Dockerfile.build Makefile.docker
