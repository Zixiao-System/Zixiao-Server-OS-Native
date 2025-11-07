# Zixiao Server OS Makefile - Dual Architecture Build System

# Default target
.PHONY: all
all: x86_64 arm64

# Architecture selection
ARCH ?= x86_64

# Directories
SRC_DIR := src
BUILD_DIR := build
INCLUDE_DIR := $(SRC_DIR)/include

# Common sources
KERNEL_LIB_C := $(SRC_DIR)/kernel/lib/string.c \
                $(SRC_DIR)/kernel/lib/printf.c

KERNEL_FS_C := $(SRC_DIR)/kernel/fs/vfs.c \
               $(SRC_DIR)/kernel/fs/initrd.c

# x86_64 Configuration
X86_64_CC := x86_64-elf-gcc
X86_64_AS := x86_64-elf-as
X86_64_LD := x86_64-elf-ld

X86_64_CFLAGS := -ffreestanding -O2 -Wall -Wextra -nostdlib -mcmodel=large \
                 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
                 -I$(INCLUDE_DIR)

X86_64_LDFLAGS := -nostdlib -T $(SRC_DIR)/arch/x86_64/linker.ld

X86_64_BOOT_S := $(SRC_DIR)/arch/x86_64/boot/boot.S
X86_64_INT_S := $(SRC_DIR)/arch/x86_64/interrupts/interrupts.S \
                $(SRC_DIR)/arch/x86_64/interrupts/gdt_flush.S

X86_64_C := $(SRC_DIR)/arch/x86_64/kernel_main.c \
            $(SRC_DIR)/arch/x86_64/drivers/vga.c \
            $(SRC_DIR)/arch/x86_64/drivers/keyboard.c \
            $(SRC_DIR)/arch/x86_64/interrupts/gdt.c \
            $(SRC_DIR)/arch/x86_64/interrupts/idt.c

X86_64_OBJS := $(BUILD_DIR)/x86_64/boot.o \
               $(BUILD_DIR)/x86_64/interrupts.o \
               $(BUILD_DIR)/x86_64/gdt_flush.o \
               $(BUILD_DIR)/x86_64/kernel_main.o \
               $(BUILD_DIR)/x86_64/vga.o \
               $(BUILD_DIR)/x86_64/keyboard.o \
               $(BUILD_DIR)/x86_64/gdt.o \
               $(BUILD_DIR)/x86_64/idt.o \
               $(BUILD_DIR)/x86_64/string.o \
               $(BUILD_DIR)/x86_64/printf.o \
               $(BUILD_DIR)/x86_64/vfs.o \
               $(BUILD_DIR)/x86_64/initrd.o

X86_64_KERNEL := $(BUILD_DIR)/zixiao-x86_64.elf
X86_64_ISO := $(BUILD_DIR)/zixiao-x86_64.iso

# ARM64 Configuration
ARM64_CC := aarch64-elf-gcc
ARM64_AS := aarch64-elf-as
ARM64_LD := aarch64-elf-ld

ARM64_CFLAGS := -ffreestanding -O2 -Wall -Wextra -nostdlib \
                -I$(INCLUDE_DIR)

ARM64_LDFLAGS := -nostdlib -T $(SRC_DIR)/arch/arm64/linker.ld

ARM64_BOOT_S := $(SRC_DIR)/arch/arm64/boot/boot.S

ARM64_C := $(SRC_DIR)/arch/arm64/kernel_main.c \
           $(SRC_DIR)/arch/arm64/drivers/uart.c \
           $(SRC_DIR)/arch/arm64/interrupts/exceptions.c

ARM64_OBJS := $(BUILD_DIR)/arm64/boot.o \
              $(BUILD_DIR)/arm64/kernel_main.o \
              $(BUILD_DIR)/arm64/uart.o \
              $(BUILD_DIR)/arm64/exceptions.o \
              $(BUILD_DIR)/arm64/string.o \
              $(BUILD_DIR)/arm64/printf.o \
              $(BUILD_DIR)/arm64/vfs.o \
              $(BUILD_DIR)/arm64/initrd.o

ARM64_KERNEL := $(BUILD_DIR)/zixiao-arm64.elf

# Build targets
.PHONY: x86_64 arm64 clean

x86_64: $(X86_64_KERNEL) $(X86_64_ISO)

arm64: $(ARM64_KERNEL)

# x86_64 Build Rules
$(BUILD_DIR)/x86_64:
	mkdir -p $(BUILD_DIR)/x86_64

$(BUILD_DIR)/x86_64/boot.o: $(X86_64_BOOT_S) | $(BUILD_DIR)/x86_64
	$(X86_64_AS) $< -o $@

$(BUILD_DIR)/x86_64/interrupts.o: $(SRC_DIR)/arch/x86_64/interrupts/interrupts.S | $(BUILD_DIR)/x86_64
	$(X86_64_AS) $< -o $@

$(BUILD_DIR)/x86_64/gdt_flush.o: $(SRC_DIR)/arch/x86_64/interrupts/gdt_flush.S | $(BUILD_DIR)/x86_64
	$(X86_64_AS) $< -o $@

$(BUILD_DIR)/x86_64/kernel_main.o: $(SRC_DIR)/arch/x86_64/kernel_main.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/vga.o: $(SRC_DIR)/arch/x86_64/drivers/vga.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/keyboard.o: $(SRC_DIR)/arch/x86_64/drivers/keyboard.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/gdt.o: $(SRC_DIR)/arch/x86_64/interrupts/gdt.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/idt.o: $(SRC_DIR)/arch/x86_64/interrupts/idt.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/string.o: $(SRC_DIR)/kernel/lib/string.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/printf.o: $(SRC_DIR)/kernel/lib/printf.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/vfs.o: $(SRC_DIR)/kernel/fs/vfs.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/x86_64/initrd.o: $(SRC_DIR)/kernel/fs/initrd.c | $(BUILD_DIR)/x86_64
	$(X86_64_CC) $(X86_64_CFLAGS) -c $< -o $@

$(X86_64_KERNEL): $(X86_64_OBJS)
	$(X86_64_LD) $(X86_64_LDFLAGS) -o $@ $^

$(X86_64_ISO): $(X86_64_KERNEL)
	mkdir -p $(BUILD_DIR)/isodir/boot/grub
	cp $(X86_64_KERNEL) $(BUILD_DIR)/isodir/boot/zixiao.elf
	echo 'set timeout=0' > $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo 'set default=0' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo 'menuentry "Zixiao OS x86_64" {' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '  multiboot2 /boot/zixiao.elf' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '  boot' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	echo '}' >> $(BUILD_DIR)/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD_DIR)/isodir 2>/dev/null || echo "Warning: grub-mkrescue not found. ISO not created."

# ARM64 Build Rules
$(BUILD_DIR)/arm64:
	mkdir -p $(BUILD_DIR)/arm64

$(BUILD_DIR)/arm64/boot.o: $(ARM64_BOOT_S) | $(BUILD_DIR)/arm64
	$(ARM64_AS) $< -o $@

$(BUILD_DIR)/arm64/kernel_main.o: $(SRC_DIR)/arch/arm64/kernel_main.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/uart.o: $(SRC_DIR)/arch/arm64/drivers/uart.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/exceptions.o: $(SRC_DIR)/arch/arm64/interrupts/exceptions.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/string.o: $(SRC_DIR)/kernel/lib/string.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/printf.o: $(SRC_DIR)/kernel/lib/printf.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/vfs.o: $(SRC_DIR)/kernel/fs/vfs.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(BUILD_DIR)/arm64/initrd.o: $(SRC_DIR)/kernel/fs/initrd.c | $(BUILD_DIR)/arm64
	$(ARM64_CC) $(ARM64_CFLAGS) -c $< -o $@

$(ARM64_KERNEL): $(ARM64_OBJS)
	$(ARM64_LD) $(ARM64_LDFLAGS) -o $@ $^

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run targets
.PHONY: run-x86_64 run-arm64

run-x86_64: x86_64
	@if [ -f $(X86_64_ISO) ]; then \
		qemu-system-x86_64 -cdrom $(X86_64_ISO) -m 512M -serial stdio; \
	else \
		echo "Error: ISO file not found. Make sure grub-mkrescue is installed."; \
		echo "You can also run the kernel directly:"; \
		echo "qemu-system-x86_64 -kernel $(X86_64_KERNEL) -m 512M -serial stdio"; \
	fi

run-arm64: arm64
	qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel $(ARM64_KERNEL) -m 512M -nographic

# Help
.PHONY: help
help:
	@echo "Zixiao Server OS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build both architectures (default)"
	@echo "  x86_64      - Build x86_64 kernel"
	@echo "  arm64       - Build ARM64 kernel"
	@echo "  clean       - Remove build artifacts"
	@echo "  run-x86_64  - Build and run x86_64 kernel in QEMU"
	@echo "  run-arm64   - Build and run ARM64 kernel in QEMU"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Requirements:"
	@echo "  x86_64: x86_64-elf-gcc, x86_64-elf-as, x86_64-elf-ld, grub-mkrescue, qemu-system-x86_64"
	@echo "  arm64:  aarch64-elf-gcc, aarch64-elf-as, aarch64-elf-ld, qemu-system-aarch64"
