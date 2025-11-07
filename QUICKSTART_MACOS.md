# 在 macOS 上快速开始 Zixiao OS

## 当前状态

你正在使用 **macOS ARM64**（Apple Silicon），这实际上是运行 ARM64 内核的最佳平台！

## 快速解决方案

### 方法 1：安装 LLVM（正在进行中）

```bash
brew install llvm
```

安装完成后，更新 PATH：

```bash
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

然后编译并运行：

```bash
make -f Makefile.native run
```

### 方法 2：安装 QEMU + 交叉编译器（推荐用于 x86_64）

```bash
# 安装 QEMU
brew install qemu

# 安装 ARM64 交叉编译器（可选，如��� Homebrew 有的话）
brew tap messense/macos-cross-toolchains
brew install aarch64-unknown-linux-gnu
```

### 方法 3：使用 Docker（最简单，适用于两个架构）

创建 Dockerfile：

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \\
    gcc-aarch64-linux-gnu \\
    gcc-x86-64-linux-gnu \\
    binutils-aarch64-linux-gnu \\
    binutils-x86-64-linux-gnu \\
    qemu-system-aarch64 \\
    qemu-system-x86 \\
    make

WORKDIR /workspace
```

构建并运行：

```bash
docker build -t zixiao-build .
docker run -v $(pwd):/workspace -it zixiao-build /bin/bash

# 在容器内
make arm64
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -m 512M -nographic
```

## 安装进度

正在安装的包：
- llvm (21.1.5)
- sqlite (3.51.0)
- z3 (4.15.4)
- python@3.14 (3.14.0_1)

## 为什么需要这些工具？

- **LLVM**: 提供 `ld.lld` 链接器，用于链接内核对象文件
- **QEMU**: 虚拟机，用于测试内核
- **交叉编译器**: 用于编译 x86_64 版本（如果需要）

## 当前编译进度

✅ 源代码编译成功
✅ 所有对象文件已生成
❌ 链接失败（缺少 ld.lld）

一旦 LLVM 安装完成，就可以成功链接并运行了！

## 预期结果

当成功运行后，你会看到：

```
========================================
  Zixiao Server OS - ARM64 Edition
========================================

Kernel loaded successfully!
Architecture: ARM64 (AArch64)
Running at EL1 (Kernel mode)

Initializing subsystems...
  [*] Initializing exception handlers...
  [*] Enabling interrupts...

All systems initialized!

UART console ready. Type something:
(Press Ctrl+A then X to exit QEMU)
```

## 下一步

LLVM 安装完成后：

```bash
# 1. 更新 PATH
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# 2. 构建并运行
make -f Makefile.native run

# 或者分步执行
make -f Makefile.native clean
make -f Makefile.native arm64
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -m 512M -nographic
```

正在等待 LLVM 安装完成...
