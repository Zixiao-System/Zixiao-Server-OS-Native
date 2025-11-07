# ARM64 Toolchain File for CMake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# 指定交叉编译工具链
set(CMAKE_C_COMPILER aarch64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-unknown-linux-gnu-g++)
set(CMAKE_ASM_COMPILER aarch64-unknown-linux-gnu-as)
set(CMAKE_AR aarch64-unknown-linux-gnu-ar)
set(CMAKE_RANLIB aarch64-unknown-linux-gnu-ranlib)

# 跳过编译器测试
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# 查找根路径
set(CMAKE_FIND_ROOT_PATH /opt/homebrew/opt/aarch64-unknown-linux-gnu)

# 调整搜索路径
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
