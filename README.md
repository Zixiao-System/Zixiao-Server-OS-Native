# Zixiao Server OS

一个支持 x86_64 和 ARM64 双架构的操作系统内核，专为服务器环境设计。

## 特性

### 已实现功能
- ✅ **双架构支持**
  - x86_64 (64位长模式)
  - ARM64 (AArch64)

- ✅ **启动系统**
  - x86_64: Multiboot2 启动 + 长模式切换
  - ARM64: EL1 启动 + 异常向量表

- ✅ **中断处理**
  - x86_64: GDT/IDT + PIC 8259A
  - ARM64: 异常向量表

- ✅ **设备驱动**
  - x86_64: VGA 文本模式 + PS/2 键盘
  - ARM64: PL011 UART

- ✅ **文件系统**
  - 虚拟文件系统 (VFS)
  - InitRD (内存文件系统)

## 系统要求

### 开发工具

#### x86_64 构建需要：
```bash
# macOS (使用 Homebrew)
brew install x86_64-elf-gcc
brew install x86_64-elf-binutils
brew install grub
brew install qemu

# Linux (Ubuntu/Debian)
sudo apt install gcc-x86-64-linux-gnu
sudo apt install binutils-x86-64-linux-gnu
sudo apt install grub-pc-bin grub-common xorriso
sudo apt install qemu-system-x86
```

#### ARM64 构建需要：
```bash
# macOS (使用 Homebrew)
brew install aarch64-elf-gcc
brew install aarch64-elf-binutils
brew install qemu

# Linux (Ubuntu/Debian)
sudo apt install gcc-aarch64-linux-gnu
sudo apt install binutils-aarch64-linux-gnu
sudo apt install qemu-system-arm
```

### macOS 用户注意

在 macOS 上，你可能需要安装交叉编译工具链：

```bash
# 安装 x86_64 交叉编译器
brew tap nativeos/i386-elf-toolchain
brew install i386-elf-binutils i386-elf-gcc

# 或者使用预编译的工具链
# 下载地址：https://github.com/nativeos/homebrew-i386-elf-toolchain
```

## 构建指南

### 构建所有架构
```bash
make
```

### 只构建特定架构
```bash
# 只构建 x86_64
make x86_64

# 只构建 ARM64
make arm64
```

### 清理构建产物
```bash
make clean
```

## 运行指南

### 运行 x86_64 版本

#### 使用脚本（推荐）
```bash
./run-x86_64.sh
```

#### 使用 Makefile
```bash
make run-x86_64
```

#### 手动运行
```bash
# 使用 ISO 镜像
qemu-system-x86_64 -cdrom build/zixiao-x86_64.iso -m 512M

# 或直接加载 ELF 内核
qemu-system-x86_64 -kernel build/zixiao-x86_64.elf -m 512M
```

**退出 QEMU**: 按 `Ctrl+C` 或在 QEMU 窗口中按 `Ctrl+Alt+Q`

### 运行 ARM64 版本

#### 使用脚本（推荐）
```bash
./run-arm64.sh
```

#### 使用 Makefile
```bash
make run-arm64
```

#### 手动运行
```bash
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -m 512M -nographic
```

**退出 QEMU**: 按 `Ctrl+A` 然后按 `X`

## 项目结构

```
Zixiao-Server-OS-Native/
├── src/
│   ├── arch/
│   │   ├── x86_64/
│   │   │   ├── boot/           # x86_64 启动代码
│   │   │   ├── interrupts/     # 中断处理 (GDT/IDT)
│   │   │   ├── drivers/        # VGA + 键盘驱动
│   │   │   ├── linker.ld       # 链接器脚本
│   │   │   └── kernel_main.c   # 内核主函数
│   │   └── arm64/
│   │       ├── boot/           # ARM64 启动代码
│   │       ├── interrupts/     # 异常处理
│   │       ├── drivers/        # UART 驱动
│   │       ├── linker.ld       # 链接器脚本
│   │       └── kernel_main.c   # 内核主函数
│   ├── kernel/
│   │   ├── lib/                # 通用库 (string, printf)
│   │   └── fs/                 # 文件系统 (VFS, InitRD)
│   └── include/
│       ├── kernel/             # 内核头文件
│       └── arch/               # 架构相关头文件
├── build/                      # 构建输出目录
├── Makefile                    # 构建系统
├── run-x86_64.sh              # x86_64 测试脚本
├── run-arm64.sh               # ARM64 测试脚本
└── README.md                  # 本文件
```

## 使用示例

### x86_64 版本

启动后，你会看到 VGA 文本模式的欢迎界面：

```
========================================
  Zixiao Server OS - x86_64 Edition
========================================

Kernel loaded successfully!
Architecture: x86_64 (Long Mode)

Initializing subsystems...
  [*] Initializing GDT...
  [*] Initializing IDT...
  [*] Initializing keyboard driver...
  [*] Enabling interrupts...

All systems initialized!

Type something on the keyboard...
```

你可以直接在键盘上输入，字符会显示在屏幕上。

### ARM64 版本

启动后，UART 控制台会显示：

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
```

输入的字符会通过 UART 回显。

## 技术细节

### x86_64 架构

- **启动流程**: Multiboot2 → 保护模式 → 长模式 → C 内核
- **内存布局**: 1MB 起始地址，使用 2MB 大页
- **中断**: 256 个 IDT 条目，PIC 重映射到 IRQ 32-47
- **显示**: VGA 文本模式 80x25，白色前景黑色背景

### ARM64 架构

- **启动流程**: 直接进入 EL1 → 设置异常向量 → C 内核
- **内存布局**: 0x40000000 起始地址 (QEMU virt)
- **中断**: 16 个异常向量 (4x4 表)
- **显示**: PL011 UART @ 0x09000000，115200 波特率

## 文件系统测试

内核包含一个简单的 InitRD 文件系统，预加载了两个测试文件：

1. `welcome.txt` - 欢迎信息
2. `readme.txt` - 系统信息

（文件系统 API 已实现，可在内核代码中使用 VFS 接口读取）

## 开发者指南

### 添加新的架构支持

1. 在 `src/arch/` 下创建新目录
2. 实现启动代码、中断处理和驱动
3. 创建链接器脚本
4. 更新 Makefile

### 添加新的设备驱动

1. 在 `src/arch/[arch]/drivers/` 下创建驱动文件
2. 实现驱动初始化和操作函数
3. 在 `kernel_main.c` 中调用初始化函数

### 调试技巧

```bash
# 在 GDB 中调试 x86_64
qemu-system-x86_64 -kernel build/zixiao-x86_64.elf -s -S &
gdb build/zixiao-x86_64.elf
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue

# 在 GDB 中调试 ARM64
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -s -S -nographic &
gdb-multiarch build/zixiao-arm64.elf
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

## 已知限制

- x86_64 版本需要 grub-mkrescue 才能创建 ISO
- 内存管理尚未实现（无堆分配器）
- 进程调度尚未实现
- 网络功能尚未实现
- 文件系统仅支持只读 InitRD

## 下一步计划

- [ ] 实现物理/虚拟内存管理
- [ ] 实现堆分配器 (kmalloc/kfree)
- [ ] 实现进程调度器
- [ ] 添加系统调用接口
- [ ] 实现可读写文件系统
- [ ] 添加网络栈
- [ ] 支持更多设备驱动

## 许可证

本项目仅供学习和研究使用。

## 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**: 这是一个教育性质的操作系统内核项目，不适用于生产环境。
