# ✅ CLion CMake 配置 - 最终解决方案

## 问题根源

CLion 的 CMake 会自动缓存编译器选择。即使修改了 CMakeLists.txt，旧的缓存仍会使用系统的 `/usr/bin/cc`。

## 🔧 解决方案：配置 CMake 选项

### 步骤 1: 打开 CMake 设置

1. **CLion** → **Settings/Preferences** (Cmd+,)
2. **Build, Execution, Deployment** → **CMake**

### 步骤 2: 编辑 Debug 配置

在 **CMake options** 字段中输入：

```
-DARCH=arm64 -DCMAKE_C_COMPILER=aarch64-unknown-linux-gnu-gcc -DCMAKE_ASM_COMPILER=aarch64-unknown-linux-gnu-as
```

或者更简洁（使用 toolchain 文件）：

```
-DCMAKE_TOOLCHAIN_FILE=toolchain-arm64.cmake
```

完整配置示例：

```
Name: Debug
Build type: Debug
Toolchain: Default
CMake options: -DARCH=arm64 -DCMAKE_C_COMPILER=aarch64-unknown-linux-gnu-gcc -DCMAKE_ASM_COMPILER=aarch64-unknown-linux-gnu-as
Build directory: cmake-build-debug
```

### 步骤 3: 应用并重新加载

1. 点击 **Apply**
2. 点击 **OK**
3. **File** → **Reload CMake Project**

## ✅ 验证配置

CMake 窗口应该显示：

```
-- Building for architecture: arm64
-- C Compiler: /opt/homebrew/bin/aarch64-unknown-linux-gnu-gcc
-- ASM Compiler: /opt/homebrew/bin/aarch64-unknown-linux-gnu-as
-- Configuring done
```

## 🎯 现在可以使用的功能

### 构建
点击 🔨 或按 **Cmd+F9**

### 运行
在 CMake 工具窗口中：
- 右键 `run` → **Build**

### 调试
1. 在 CMake 工具窗口中右键 `debug` → **Build**（启动 QEMU GDB 服务器）
2. 使用 Remote Debug 配置连接

## 📸 配置截图指南

### CMake 设置位置：
```
CLion
  └─ Settings (Cmd+,)
      └─ Build, Execution, Deployment
          └─ CMake
              └─ Profiles
                  └─ Debug [选中这个]
                      ├─ Name: Debug
                      ├─ Build type: Debug
                      ├─ CMake options: [在这里粘贴上面的命令行选项]
                      └─ Build directory: cmake-build-debug
```

## 🚨 常见问题

### Q: 仍然显示 "/usr/bin/cc"

**A**:
1. 删除 `cmake-build-debug` 文件夹
2. **File** → **Reload CMake Project**

### Q: "aarch64-unknown-linux-gnu-gcc not found"

**A**: 确保交叉编译器在 PATH 中：
```bash
which aarch64-unknown-linux-gnu-gcc
# 应该输出: /opt/homebrew/bin/aarch64-unknown-linux-gnu-gcc
```

### Q: 编译选项中出现 "-arch arm64"

**A**: 这说明 CMake 仍在使用系统编译器。确保：
1. CMake options 中包含 `-DCMAKE_C_COMPILER=aarch64-unknown-linux-gnu-gcc`
2. 已删除旧的 cmake-build-debug
3. 已重新加载 CMake 项目

## 🎬 完整工作流程

1. **配置一次**（按上述步骤）
2. **编辑代码**
3. **Cmd+F9** 构建
4. **CMake 窗口 → run** 运行
5. **调试时**：先运行 `debug` 目标，再启动 Remote Debug

## 💡 专业提示

创建多个 CMake 配置：

- **Debug**: 调试版本（-O0 -g）
- **Release**: 发布版本（-O2）
- **x86_64**: 切换到 x86_64 架构

在 CMake 设置中点击 **+** 可以添加新配置，每个配置可以有不同的 `ARCH` 参数。

---

**现在你的 CLion 应该可以完美工作了！** ✨

构建输出应该显示：
```
[100%] Built target zixiao-arm64.elf
```

而不是编译错误！
