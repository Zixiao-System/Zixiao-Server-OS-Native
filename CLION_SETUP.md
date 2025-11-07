# CLion 配置和使用指南

本文档说明如何在 CLion 中配置和调试 Zixiao OS。

## 1. 首次配置

### 步骤 1: 重新加载 CMake 项目

1. 打开 CLion
2. File → Reload CMake Project
3. 等待 CMake 配置完成

### 步骤 2: 配置 CMake 选项

1. 打开 **Settings/Preferences** (Cmd+,)
2. 导航到 **Build, Execution, Deployment → CMake**
3. 添加两个配置：

#### ARM64 配置
- **Name**: ARM64-Debug
- **Build type**: Debug
- **CMake options**: `-DARCH=arm64`
- **Build directory**: `cmake-build-arm64`

#### x86_64 配置（可选）
- **Name**: x86_64-Debug
- **Build type**: Debug
- **CMake options**: `-DARCH=x86_64`
- **Build directory**: `cmake-build-x86_64`

## 2. 构建项目

### 使用 CLion UI

1. 在工具栏选择配置：`ARM64-Debug` 或 `x86_64-Debug`
2. 点击 Build 按钮（锤子图标）或按 **Cmd+F9**

### 使用 CMake 目标

1. 打开 **CMake** 工具窗口（View → Tool Windows → CMake）
2. 展开目标列表
3. 双击 `zixiao-arm64.elf` 或 `zixiao-x86_64.elf` 进行构建

## 3. 运行内核

### 方法 1: 使用自定义目标

1. 打开 **CMake** 工具窗口
2. 找到 `run` 目标
3. 右键点击 → **Build**

这会自动：
- 编译内核
- 在 QEMU 中运行

### 方法 2: 创建运行配置

1. 点击工具栏的 **Edit Configurations...**
2. 点击 **+** → **CMake Application**
3. 配置如下：

**ARM64 运行配置**:
- **Name**: Run Zixiao ARM64
- **Target**: run
- **Executable**: `cmake-build-arm64/zixiao-arm64.elf`

4. 点击 **OK**
5. 现在可以通过工具栏直接运行（**Ctrl+R** 或 **Shift+F10**）

## 4. 调试配置

### 步骤 1: 启动 QEMU GDB Server

有两种方式：

#### 方式 A: 使用 CMake 目标
1. 打开 **CMake** 工具窗口
2. 找到 `debug` 目标
3. 右键点击 → **Build**

这会启动 QEMU 并等待 GDB 连接（监听在 `localhost:1234`）

#### 方式 B: 手动启动
```bash
# ARM64
qemu-system-aarch64 -M virt -cpu cortex-a57 \
  -kernel cmake-build-arm64/zixiao-arm64.elf \
  -m 512M -nographic -s -S

# x86_64
qemu-system-x86_64 -kernel cmake-build-x86_64/zixiao-x86_64.elf \
  -m 512M -serial stdio -s -S
```

### 步骤 2: 配置 GDB 远程调试

1. 点击 **Edit Configurations...**
2. 点击 **+** → **Remote Debug**
3. 配置如下：

**ARM64 调试配置**:
- **Name**: Debug Zixiao ARM64
- **'target remote' args**: `localhost:1234`
- **Symbol file**: `cmake-build-arm64/zixiao-arm64.elf`
- **Sysroot**: (留空)
- **Path mappings**: (留空)

**GDB 路径**:
- macOS: 使用 `lldb` 或安装 `gdb-multiarch`
  ```bash
  brew install gdb
  ```

### 步骤 3: 开始调试

1. 首先运行 `debug` CMake 目标启动 QEMU
2. 然后从工具栏选择 **Debug Zixiao ARM64** 配置
3. 点击 Debug 按钮（虫子图标）或按 **Ctrl+D**

### 调试技巧

#### 设置断点
- 在源代码中点击行号旁边的空白区域
- 或按 **Cmd+F8**

#### 常用断点位置
- `src/arch/arm64/kernel_main.c:kernel_main` - 内核入口
- `src/arch/arm64/drivers/uart.c:console_putchar` - 字符输出
- `src/arch/arm64/interrupts/exceptions.c:arm64_irq_handler` - 中断处理

#### 调试命令
- **F8**: Step Over
- **F7**: Step Into
- **Shift+F8**: Step Out
- **Cmd+Alt+R**: Resume Program
- **Cmd+F2**: Stop

## 5. 终端输出

### ARM64
QEMU 运行在 `-nographic` 模式，输出会显示在：
- CLion 的 **Run** 工具窗口（如果通过 CLion 运行）
- 启动 QEMU 的终端窗口

退出 QEMU: **Ctrl+A**, 然后按 **X**

### x86_64
QEMU 会打开一个窗口显示 VGA 输出。

## 6. 常见问题

### Q: CMake 找不到编译器
**A**: 确保 LLVM 已安装并在 PATH 中：
```bash
brew install llvm
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
```

### Q: 链接失败："ld: unknown option"
**A**: 检查是否使用了正确的链接器。CMakeLists.txt 会自动检测 LLVM 的 `ld.lld`。

### Q: GDB 连接失败
**A**:
1. 确保 QEMU 在 GDB 模式下运行（`-s -S` 选项）
2. 检查端口 1234 是否被占用
3. 对于 ARM64，可能需要使用 `gdb-multiarch` 而不是普通的 `gdb`

### Q: 找不到 "run" 或 "debug" 目标
**A**: 重新加载 CMake 项目（File → Reload CMake Project）

### Q: macOS 上 GDB 签名问题
**A**: macOS 需要对 GDB 进行代码签名。推荐使用 LLDB：
```bash
# 使用 LLDB 调试
lldb cmake-build-arm64/zixiao-arm64.elf
(lldb) gdb-remote localhost:1234
(lldb) b kernel_main
(lldb) c
```

## 7. 代码导航技巧

### 快速导航
- **Cmd+O**: 查找文件
- **Cmd+Shift+O**: 查找符号（函数、变量等）
- **Cmd+B**: 跳转到定义
- **Cmd+Alt+B**: 跳转到实现
- **Cmd+[** / **Cmd+]**: 前进/后退

### 查找用法
- **Alt+F7**: 查找所有用法
- **Cmd+Alt+F7**: 在工具窗口中显示用法

### 重构
- **Shift+F6**: 重命名
- **Cmd+Alt+M**: 提取方法

## 8. 推荐的工作流程

### 开发流程
1. 编写代码
2. **Cmd+S** 保存
3. **Cmd+F9** 构建
4. 运行 `run` CMake 目标测试
5. 如果有 bug，使用 `debug` 目标调试

### 调试流程
1. 设置断点
2. 运行 `debug` CMake 目标（启动 QEMU GDB 服务器）
3. 启动 GDB 远程调试配置
4. 使用调试工具栏控制执行
5. 查看变量、寄存器、内存

## 9. 键盘快捷键总结

| 操作 | macOS | 说明 |
|------|-------|------|
| 构建 | Cmd+F9 | 编译当前配置 |
| 运行 | Ctrl+R | 运行当前配置 |
| 调试 | Ctrl+D | 启动调试 |
| 停止 | Cmd+F2 | 停止运行/调试 |
| 切换断点 | Cmd+F8 | 设置/取消断点 |
| Step Over | F8 | 单步跳过 |
| Step Into | F7 | 单步进入 |
| Resume | Cmd+Alt+R | 继续执行 |

---

## 附录：完整示例配置

### .idea/runConfigurations/Run_ARM64.xml
CLion 会自动生成这些文件，但你也可以手动创建：

```xml
<component name="ProjectRunConfigurationManager">
  <configuration default="false" name="Run ARM64" type="CMakeRunConfiguration" factoryName="Application">
    <config>
      <option name="BUILD_TARGET" value="run" />
      <option name="REDIRECT_INPUT" value="false" />
      <option name="EMULATE_TERMINAL" value="true" />
      <option name="PASS_PARENT_ENVS" value="true" />
      <option name="PROJECT_NAME" value="Zixiao_OS" />
      <option name="TARGET_NAME" value="run" />
    </config>
  </configuration>
</component>
```

现在你可以在 CLion 中完整地开发、构建、运行和调试 Zixiao OS 了！
