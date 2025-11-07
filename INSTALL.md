# 工具链安装指南

本文档说明如何在不同平台上安装 Zixiao OS 的开发工具链。

## macOS (Apple Silicon / ARM64)

### 方法 1: 使用 Homebrew (推荐)

#### 1. 安装 QEMU
```bash
brew install qemu
```

#### 2. 安装交叉编译工具链

对于 x86_64:
```bash

# 方法 A: 使用第三方 tap
brew tap nativeos/i386-elf-toolchain
brew install x86_64-elf-gcc
brew install x86_64-elf-binutils

# 方法 B: 如果上面不可用，使用 i386-elf 工具链
brew install i386-elf-gcc
brew install i386-elf-binutils
```

对于 ARM64 (本机架构，可能已安装):
```bash
# 检查是否已有 gcc
gcc --version

# 或安装交叉编译器
brew install aarch64-elf-gcc
brew install aarch64-elf-binutils
```

#### 3. 安装 GRUB (可选，用于创建 x86_64 ISO)
```bash
brew install grub
```

### 方法 2: 从源码编译工具链

如果 Homebrew 没有预编译的包，你需要自己编译交叉编译器。

#### 编译 x86_64-elf-gcc

```bash
# 创建工作目录
mkdir -p ~/cross
cd ~/cross

# 设置环境变量
export PREFIX="$HOME/cross/x86_64-elf"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# 下载 binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.xz
tar xf binutils-2.40.tar.xz
mkdir build-binutils
cd build-binutils
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(sysctl -n hw.ncpu)
make install
cd ..

# 下载 GCC
wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
tar xf gcc-13.2.0.tar.xz
mkdir build-gcc
cd build-gcc
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j$(sysctl -n hw.ncpu) all-gcc
make -j$(sysctl -n hw.ncpu) all-target-libgcc
make install-gcc
make install-target-libgcc

# 添加到 PATH
echo 'export PATH="$HOME/cross/x86_64-elf/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

#### 编译 aarch64-elf-gcc

```bash
export PREFIX="$HOME/cross/aarch64-elf"
export TARGET=aarch64-elf

# Binutils
mkdir build-binutils-aarch64
cd build-binutils-aarch64
../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$(sysctl -n hw.ncpu)
make install
cd ..

# GCC
mkdir build-gcc-aarch64
cd build-gcc-aarch64
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j$(sysctl -n hw.ncpu) all-gcc
make -j$(sysctl -n hw.ncpu) all-target-libgcc
make install-gcc
make install-target-libgcc

# 添加到 PATH
echo 'export PATH="$HOME/cross/aarch64-elf/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### 方法 3: 使用 Docker (最简单)

创建一个 Docker 容器来编译：

```bash
# 拉取包含工具链的镜像
docker pull randomdude/gcc-cross-x86_64-elf

# 运行编译
docker run -v $(pwd):/work randomdude/gcc-cross-x86_64-elf make x86_64
```

## Linux (Ubuntu/Debian)

```bash
# 更新包列表
sudo apt update

# 安装 x86_64 工具链
sudo apt install gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu

# 安装 ARM64 工具链
sudo apt install gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu

# 安装 QEMU
sudo apt install qemu-system-x86 qemu-system-arm

# 安装 GRUB (创建 ISO)
sudo apt install grub-pc-bin grub-common xorriso mtools
```

## Linux (Arch)

```bash
# x86_64 工具链
sudo pacman -S x86_64-elf-gcc x86_64-elf-binutils

# ARM64 工具链
sudo pacman -S aarch64-linux-gnu-gcc aarch64-linux-gnu-binutils

# QEMU
sudo pacman -S qemu-system-x86 qemu-system-aarch64

# GRUB
sudo pacman -S grub xorriso mtools
```

## 验证安装

安装完成后，运行以下命令验证：

```bash
# 检查 x86_64 工具链
x86_64-elf-gcc --version
x86_64-elf-as --version
x86_64-elf-ld --version

# 检查 ARM64 工具链
aarch64-elf-gcc --version
aarch64-elf-as --version
aarch64-elf-ld --version

# 检查 QEMU
qemu-system-x86_64 --version
qemu-system-aarch64 --version

# 检查 GRUB (可选)
grub-mkrescue --version
```

## 使用本机编译器 (临时方案)

如果你在 ARM64 macOS 上只想测试 ARM64 版本，可以修改 Makefile 使用系统自带的 clang：

```makefile
# 在 Makefile 中修改 ARM64 配置
ARM64_CC := clang
ARM64_AS := clang
ARM64_LD := ld

ARM64_CFLAGS := -target aarch64-none-elf -ffreestanding -O2 -Wall -Wextra -nostdlib -I$(INCLUDE_DIR)
```

然后只编译 ARM64 版本：
```bash
make arm64
./run-arm64.sh
```

## 常见问题

### Q: Homebrew 找不到 x86_64-elf-gcc
A: 尝试使用 i686-elf-gcc 或从源码编译，或使用 Docker 方案。

### Q: macOS 上 grub-mkrescue 不工作
A: 这是已知问题。可以跳过 ISO 创建，直接使用 ELF 文件：
```bash
qemu-system-x86_64 -kernel build/zixiao-x86_64.elf
```

### Q: Apple Silicon Mac 能运行 x86_64 QEMU 吗？
A: 可以！QEMU 会使用软件模拟，速度会慢一些但能正常工作。

### Q: 编译时报错 "command not found"
A: 确保工具链已添加到 PATH。运行 `echo $PATH` 检查。

## 推荐的快速开始方案

**对于 macOS Apple Silicon 用户**：

1. 安装 QEMU:
   ```bash
   brew install qemu
   ```

2. 先测试 ARM64 版本（使用系统 clang）:
   ```bash
   # 修改 Makefile 使用 clang (见上文)
   make arm64
   ./run-arm64.sh
   ```

3. 如果需要 x86_64，安装 Docker 并使用容器编译

**对于 Linux 用户**：
直接使用包管理器安装所有工具，最简单。

---

安装完成后，回到主 README.md 继续构建和运行项目！
