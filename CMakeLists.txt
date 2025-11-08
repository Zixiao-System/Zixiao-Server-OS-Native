cmake_minimum_required(VERSION 3.20)

# 设置架构（默认 ARM64）
if(NOT DEFINED ARCH)
    set(ARCH "arm64" CACHE STRING "Target architecture: arm64 or x86_64")
endif()

message(STATUS "Building for architecture: ${ARCH}")

# 根据架构选择配置
if(ARCH STREQUAL "arm64")
    # ARM64 交叉编译配置
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)

    # 查找交叉编译器
    find_program(AARCH64_GCC aarch64-unknown-linux-gnu-gcc)
    if(AARCH64_GCC)
        set(CMAKE_C_COMPILER aarch64-unknown-linux-gnu-gcc)
        set(CMAKE_ASM_COMPILER aarch64-unknown-linux-gnu-as)
        set(CMAKE_AR aarch64-unknown-linux-gnu-ar)
        set(CMAKE_RANLIB aarch64-unknown-linux-gnu-ranlib)
    else()
        message(FATAL_ERROR "ARM64 cross-compiler not found. Install with: brew install aarch64-unknown-linux-gnu")
    endif()

    # 禁用编译器检查（交叉编译）
    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)

elseif(ARCH STREQUAL "x86_64")
    # x86_64 配置
    set(CMAKE_SYSTEM_NAME Generic)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)

    find_program(X86_64_GCC x86_64-elf-gcc)
    if(X86_64_GCC)
        set(CMAKE_C_COMPILER x86_64-elf-gcc)
        set(CMAKE_ASM_COMPILER x86_64-elf-as)
    else()
        message(WARNING "x86_64-elf-gcc not found.")
    endif()

    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)
endif()

# 项目定义
project(Zixiao_OS C ASM)

# 设置 C 标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 包含目录
include_directories(src/include)

# 禁用标准库
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding -nostdlib -fno-builtin -fno-stack-protector")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

# 通用源文件
set(KERNEL_LIB_SOURCES
    src/kernel/lib/string.c
    src/kernel/lib/printf.c
)

set(KERNEL_MM_SOURCES
    src/kernel/mm/pmm.c
    src/kernel/mm/kmalloc.c
)

set(KERNEL_SCHED_SOURCES
    src/kernel/scheduler/sched.c
    src/kernel/scheduler/task.c
    src/kernel/scheduler/idle.c
    src/kernel/scheduler/test_tasks.c
)

set(KERNEL_FS_SOURCES
    src/kernel/fs/vfs.c
    src/kernel/fs/initrd.c
)

# 根据架构选择源文件
if(ARCH STREQUAL "arm64")
    # ARM64 源文件
    set(ARCH_SOURCES
        src/arch/arm64/boot/boot.S
        src/arch/arm64/scheduler/context_switch.S
        src/arch/arm64/kernel_main.c
        src/arch/arm64/drivers/uart.c
        src/arch/arm64/interrupts/exceptions.c
        src/arch/arm64/interrupts/gic.c
        src/arch/arm64/interrupts/timer.c
        src/arch/arm64/mm/mmu.c
    )

    # 链接器脚本
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/arch/arm64/linker.ld)

    # 链接选项
    set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -T ${LINKER_SCRIPT}")

    # 输出文件名
    set(KERNEL_OUTPUT zixiao-arm64.elf)

    # QEMU 运行命令
    set(QEMU_CMD qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel ${KERNEL_OUTPUT} -m 512M -nographic)

elseif(ARCH STREQUAL "x86_64")
    # x86_64 特定编译选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2")

    # x86_64 源文件
    set(ARCH_SOURCES
        src/arch/x86_64/boot/boot.S
        src/arch/x86_64/scheduler/context_switch.S
        src/arch/x86_64/kernel_main.c
        src/arch/x86_64/drivers/vga.c
        src/arch/x86_64/drivers/keyboard.c
        src/arch/x86_64/interrupts/gdt.c
        src/arch/x86_64/interrupts/gdt_flush.S
        src/arch/x86_64/interrupts/idt.c
        src/arch/x86_64/interrupts/interrupts.S
        src/arch/x86_64/interrupts/timer.c
        src/arch/x86_64/mm/mmu.c
    )

    # 链接器脚本
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/arch/x86_64/linker.ld)

    # 链接选项
    set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -T ${LINKER_SCRIPT}")

    # 输出文件名
    set(KERNEL_OUTPUT zixiao-x86_64.elf)

    # QEMU 运行命令
    set(QEMU_CMD qemu-system-x86_64 -kernel ${KERNEL_OUTPUT} -m 512M -serial stdio)

else()
    message(FATAL_ERROR "Unsupported architecture: ${ARCH}. Use 'arm64' or 'x86_64'")
endif()

# 创建可执行文件
add_executable(${KERNEL_OUTPUT}
    ${ARCH_SOURCES}
    ${KERNEL_LIB_SOURCES}
    ${KERNEL_MM_SOURCES}
    ${KERNEL_SCHED_SOURCES}
    ${KERNEL_FS_SOURCES}
)

# 设置输出目录
set_target_properties(${KERNEL_OUTPUT} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    OUTPUT_NAME ${KERNEL_OUTPUT}
    SUFFIX ""
)

# 链接设置
set_target_properties(${KERNEL_OUTPUT} PROPERTIES
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# 自定义目标：运行 QEMU
add_custom_target(run
    COMMAND ${QEMU_CMD}
    DEPENDS ${KERNEL_OUTPUT}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running ${KERNEL_OUTPUT} in QEMU"
)

# 自定义目标：调试（GDB）
if(ARCH STREQUAL "arm64")
    add_custom_target(debug
        COMMAND qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel ${KERNEL_OUTPUT} -m 512M -nographic -s -S
        DEPENDS ${KERNEL_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Starting QEMU with GDB server on :1234"
    )
elseif(ARCH STREQUAL "x86_64")
    add_custom_target(debug
        COMMAND qemu-system-x86_64 -kernel ${KERNEL_OUTPUT} -m 512M -serial stdio -s -S
        DEPENDS ${KERNEL_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Starting QEMU with GDB server on :1234"
    )
endif()

# 清理
set_directory_properties(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_BINARY_DIR}/${KERNEL_OUTPUT}"
)

# 打印配置信息
message(STATUS "===========================================")
message(STATUS "Zixiao OS Configuration")
message(STATUS "===========================================")
message(STATUS "Architecture: ${ARCH}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "ASM Compiler: ${CMAKE_ASM_COMPILER}")
message(STATUS "Linker Script: ${LINKER_SCRIPT}")
message(STATUS "Output: ${KERNEL_OUTPUT}")
message(STATUS "QEMU Command: ${QEMU_CMD}")
message(STATUS "===========================================")
