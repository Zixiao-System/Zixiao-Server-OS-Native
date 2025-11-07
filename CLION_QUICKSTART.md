# CLion 快速开始

## 🚀 当前状态

✅ LLVM 已安装
⏳ ARM64 交叉编译器正在安装...

## 📝 在 CLion 中的配置步骤

### 1. 重新加载 CMake 项目

在 CLion 中：
1. 菜单栏: **File** → **Reload CMake Project**
2. 或点击 CMake 工具窗口顶部的刷新按钮

### 2. 选择构建配置

工具栏会显示：
```
ARM64-Debug | zixiao-arm64.elf
```

### 3. 构建内核

点击锤子图标 🔨 或按 `Cmd+F9`

###  4. 运行内核

#### 方式 A: 使用 CMake 目标
1. 打开 **CMake** 工具窗口 (View → Tool Windows → CMake)
2. 找到 `run` 目标
3. 右键 → **Build**

#### 方式 B: 使用终端
```bash
cd cmake-build-arm64
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel zixiao-arm64.elf -m 512M -nographic
```

退出: `Ctrl+A`, 然后按 `X`

## 🐛 调试

### 步骤 1: 启动 GDB 服务器
终端运行:
```bash
# 启动 QEMU with GDB
cd cmake-build-arm64
qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel zixiao-arm64.elf -m 512M -nographic -s -S
```

### 步骤 2: 在 CLion 中配置远程调试
1. **Run** → **Edit Configurations...**
2. **+** → **Remote Debug**
3. 配置:
   - Name: `Debug ARM64`
   - Target remote: `localhost:1234`
   - Symbol file: `cmake-build-arm64/zixiao-arm64.elf`
4. **OK**

### 步骤 3: 开始调试
1. 点击调试按钮 🐛 或按 `Ctrl+D`
2. CLion 会连接到 QEMU
3. 设置断点并开始调试！

## 常用快捷键

| 操作 | 快捷键 |
|------|--------|
| 构建 | Cmd+F9 |
| 运行 | Ctrl+R |
| 调试 | Ctrl+D |
| 设置断点 | Cmd+F8 |
| 单步跳过 | F8 |
| 单步进入 | F7 |

## 💡 如果遇到编译错误

交叉编译器安装完成后:
1. 重新加载 CMake 项目
2. 清理并重新构建: **Build** → **Clean** → **Rebuild**

## 📚 完整文档

查看 `CLION_SETUP.md` 了解详细配置。
