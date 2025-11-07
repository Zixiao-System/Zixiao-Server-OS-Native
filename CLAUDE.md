# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Zixiao Server OS is a dual-architecture operating system kernel supporting both x86_64 and ARM64 (AArch64). This is a bare-metal kernel with:
- Custom bootloaders (Multiboot2 for x86_64, EL1 setup for ARM64)
- Hardware interrupt handling (GDT/IDT for x86_64, exception vectors for ARM64)
- Device drivers (VGA/PS2 keyboard for x86_64, PL011 UART for ARM64)
- Virtual File System (VFS) with InitRD support
- No standard library dependencies (freestanding environment)

## Build System

### Prerequisites
- **ARM64**: `aarch64-unknown-linux-gnu-gcc` (installed via Homebrew on macOS)
- **x86_64**: `x86_64-elf-gcc` (optional, for x86_64 builds)
- **QEMU**: `qemu-system-aarch64` and `qemu-system-x86_64`

### Build Commands

```bash
# Build ARM64 kernel (native on ARM64 macOS)
make -f Makefile.native arm64

# Build both architectures (requires cross-compilers)
make

# Clean build artifacts
make clean

# Run ARM64 kernel in QEMU
./run-arm64.sh
# Or manually:
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -m 512M -nographic
# Exit QEMU: Ctrl+A then X
```

### CLion/CMake Build

CMake requires explicit compiler specification to avoid using system clang:

```bash
# Configure from command line
cmake -B cmake-build-debug -DARCH=arm64 \
  -DCMAKE_C_COMPILER=aarch64-unknown-linux-gnu-gcc \
  -DCMAKE_ASM_COMPILER=aarch64-unknown-linux-gnu-as

# Build
cmake --build cmake-build-debug
```

**CLion Settings**: In Settings → CMake, add to "CMake options":
```
-DARCH=arm64 -DCMAKE_C_COMPILER=aarch64-unknown-linux-gnu-gcc -DCMAKE_ASM_COMPILER=aarch64-unknown-linux-gnu-as
```

After changing CMake settings, delete `cmake-build-debug/` and reload CMake project.

### Debugging with GDB

```bash
# Terminal 1: Start QEMU with GDB server
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel build/zixiao-arm64.elf -m 512M -nographic -s -S

# Terminal 2: Connect with GDB
gdb-multiarch build/zixiao-arm64.elf
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

Or use the provided GDB init script:
```bash
gdb-multiarch -x .gdbinit-arm64
```

## Architecture-Specific Code Organization

The codebase is organized with shared and architecture-specific components:

### Shared Components (`src/kernel/`, `src/include/`)
- **lib/**: `string.c`, `printf.c` - custom implementations (no libc)
- **fs/**: `vfs.c`, `initrd.c` - file system abstraction and RAM filesystem
- **include/kernel/**: Type definitions, function declarations

### Architecture Directories (`src/arch/{x86_64,arm64}/`)
Each architecture has:
- **boot/**: Assembly bootstrap code
  - x86_64: Multiboot2 header → protected mode → long mode → C entry
  - ARM64: EL1 setup → exception vectors → C entry
- **interrupts/**: Interrupt/exception handling
  - x86_64: GDT/IDT setup, ISR/IRQ handlers, PIC programming
  - ARM64: Exception vector table (16 entries: 4 exception levels × 4 types)
- **drivers/**: Hardware-specific I/O
  - x86_64: VGA text mode (0xB8000), PS/2 keyboard (port 0x60/0x64)
  - ARM64: PL011 UART (0x09000000 on QEMU virt)
- **kernel_main.c**: Architecture-specific entry point called from assembly
- **linker.ld**: Memory layout script

### Key Architecture Differences

**Startup**:
- x86_64 requires setting up paging before entering long mode
- ARM64 boots directly at EL1 with identity mapping

**Interrupts**:
- x86_64: Push registers → call C handler → send EOI to PIC → pop registers → `iretq`
- ARM64: Exception vectors branch to handlers → save/restore all 30 registers → `eret`

**Console I/O**:
- x86_64: Direct VGA buffer writes at 0xB8000
- ARM64: MMIO UART reads/writes at 0x09000000

## Important Constraints

### No Standard Library
All code is freestanding (`-ffreestanding -nostdlib`). Cannot use:
- Standard headers like `<stdio.h>`, `<stdlib.h>`
- Functions like `malloc()`, `printf()` (we implement our own)
- GCC builtins are OK: `__builtin_va_list`, `__builtin_memcpy`, etc.

### Kernel Space Only
- No user mode, no syscalls (yet)
- Everything runs at highest privilege (EL1 for ARM64, Ring 0 for x86_64)
- No memory protection between components

### Hardware Dependencies
- x86_64 code assumes QEMU or BIOS-compatible hardware
- ARM64 code targets QEMU virt machine (`-M virt`)
  - UART base: 0x09000000
  - Load address: 0x40000000

## Common Development Patterns

### Adding a New Function to Shared Library
1. Declare in `src/include/kernel/string.h` (or create new header)
2. Implement in `src/kernel/lib/*.c`
3. Both architectures automatically link against it

### Adding Architecture-Specific Code
1. Implement in `src/arch/{arch}/`
2. If exposing to other modules, add declaration to `src/include/arch/`
3. Update `Makefile` (and `CMakeLists.txt` if using CLion)

### Debugging Boot Issues
- x86_64: Check Multiboot2 magic number, page table setup
- ARM64: Check exception level (should be EL1), verify load address
- Use QEMU `-d int,cpu_reset` for detailed logging
- Check linker script memory addresses match QEMU machine layout

### Memory Layout
- x86_64: Starts at 1MB (0x100000), identity mapped first 2MB
- ARM64: Starts at 0x40000000 (QEMU virt), no paging yet
- Stack grows downward from end of kernel

## Testing

No automated tests currently. Manual testing workflow:
1. Build kernel
2. Run in QEMU
3. Verify boot messages appear
4. Test keyboard input (x86_64) or UART echo (ARM64)

Expected output on successful boot:
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

## File References

Critical files to understand the system:
- `src/arch/arm64/boot/boot.S` - ARM64 entry point and exception vectors
- `src/arch/arm64/kernel_main.c` - Main kernel initialization sequence
- `src/kernel/lib/printf.c` - Custom printf using va_args
- `src/include/kernel/types.h` - Type definitions (no stdint.h)

Documentation:
- `CLION_FINAL_SETUP.md` - Detailed CLion configuration instructions
- `PROJECT_OVERVIEW.md` - Complete file listing and statistics
- `INSTALL.md` - Toolchain installation guide

## Current Limitations & Known Issues

- x86_64 requires `grub-mkrescue` for ISO creation (may not work on macOS)
- No heap allocator (no `kmalloc`/`kfree`)
- VFS is read-only (InitRD only)
- Single-core only (no SMP)
- No network stack
- initrd.c has unused parameter warnings (intentional for VFS interface compatibility)