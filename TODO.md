# Zixiao Server OS - TODO List

## 玉衡 (Yuheng) 调度器开发路线图

### ✅ 已完成 (2025-01-08)

- [x] 基础调度器框架
  - Round-Robin (RR) 调度实现
  - 任务创建、切换、退出机制
  - ARM64 上下文切换 (context_switch.S)
  - 优先级队列管理 (链表实现)
  - 空闲任务 (idle task) 支持

- [x] 核心 Bug 修复
  - 修复编译器生成浮点指令问题 (添加 -mgeneral-regs-only)
  - 修复 cpu_context 结构体字段顺序 (fp, pc, sp)
  - 修复 CPU_CONTEXT_OFFSET 计算 (72 bytes)

- [x] CFS 准备工作
  - 在 task_struct 中添加 CFS 字段 (vruntime, exec_start, weight, sum_exec_runtime)
  - 添加调度器时钟 (scheduler_clock)
  - 预留 CFS 辅助函数骨架 (priority_to_weight, update_curr_runtime, check_preempt_curr)

### 🔄 进行中

无

### 📋 待完成任务

#### 1. 实现完整的 CFS vruntime 计算逻辑
**优先级**: 高
**预估工作量**: 2-3 天

**任务内容**:
- [ ] 实现 `priority_to_weight()` 权重映射表
  - 参考 Linux kernel prio_to_weight[] (基于 nice 值)
  - 将 0-9 优先级映射到合理的权重值

- [ ] 实现 `update_curr_runtime()` vruntime 计算
  ```c
  vruntime += delta * (NICE_0_WEIGHT / task->weight)
  ```
  - 在 scheduler_tick() 中调用
  - 在 schedule() 切换时更新

- [ ] 实现 `check_preempt_curr()` 抢占逻辑
  ```c
  if (next->vruntime + threshold < curr->vruntime)
      return true;
  ```
  - 定义合理的抢占阈值 (如 sysctl_sched_wakeup_granularity)

- [ ] 在 task_ready() 中设置 exec_start
- [ ] 在 schedule() 中维护 vruntime 最小值 (min_vruntime)

**验收标准**:
- 高优先级任务获得更少 vruntime 增量
- 低优先级任务 vruntime 增长更快
- vruntime 最小的任务优先运行

---

#### 2. 将 runqueue 从链表迁移到红黑树
**优先级**: 高
**预估工作量**: 3-4 天

**任务内容**:
- [ ] 实现通用红黑树 (rbtree) 数据结构
  - 文件: `src/kernel/lib/rbtree.c`, `src/include/kernel/rbtree.h`
  - API: `rb_insert()`, `rb_delete()`, `rb_first()`, `rb_next()`
  - 基于 vruntime 作为 key 排序

- [ ] 修改 runqueue 数据结构
  ```c
  typedef struct runqueue {
      struct rb_root tasks_timeline;  // 红黑树根
      struct rb_node *rb_leftmost;    // 缓存最左节点 (vruntime 最小)
      uint64_t min_vruntime;          // 全局最小 vruntime
      uint32_t nr_running;            // 运行队列任务数
  } runqueue_t;
  ```

- [ ] 在 task_struct 中添加红黑树节点
  ```c
  struct rb_node run_node;  // 用于插入 runqueue
  ```

- [ ] 重构调度器函数
  - `runqueue_enqueue()`: O(log n) 插入
  - `runqueue_dequeue()`: O(log n) 删除
  - `runqueue_pick_next()`: O(1) 获取 leftmost

**验收标准**:
- 所有操作复杂度符合预期
- 大量任务时性能提升明显
- vruntime 顺序始终正确

---

#### 3. 实现优先级继承和实时调度增强
**优先级**: 中
**预估工作量**: 2-3 天

**任务内容**:
- [ ] 完善 SCHED_FIFO 实现
  - 不使用 time_slice (运行到主动让出或阻塞)
  - 严格优先级调度

- [ ] 完善 SCHED_RR 实现
  - 使用 time_slice 轮转
  - 同优先级任务轮流执行

- [ ] 实现优先级继承 (Priority Inheritance)
  - 当高优先级任务等待低优先级任务持有的锁时
  - 临时提升低优先级任务的优先级
  - 释放锁后恢复原优先级

- [ ] 添加实时任务统计
  - 实时任务 CPU 占用率限制
  - 防止实时任务饿死普通任务

**依赖**:
- 需要先实现互斥锁 (mutex) 机制

**验收标准**:
- FIFO 任务按优先级严格执行
- RR 任务同优先级轮转
- 优先级反转问题得到解决

---

#### 4. 移植到 x86_64 架构
**优先级**: 低
**预估工作量**: 2-3 天

**任务内容**:
- [ ] 实现 x86_64 上下文切换
  - 文件: `src/arch/x86_64/scheduler/context_switch.S`
  - 保存/恢复 callee-saved 寄存器: rbx, rbp, r12-r15, rsp, rip

- [ ] 定义 x86_64 的 cpu_context 结构
  ```c
  typedef struct cpu_context {
      uint64_t rbx, rbp, r12, r13, r14, r15;
      uint64_t rsp, rip;
  } cpu_context_t;
  ```

- [ ] 实现 x86_64 的 arch_setup_task_context()
  - 初始化任务栈
  - 设置 rip 指向任务入口函数

- [ ] 更新 Makefile 和 CMakeLists.txt
  - 添加 x86_64 调度器源文件
  - 确保两个架构共享通用调度逻辑

**验收标准**:
- x86_64 版本通过所有 ARM64 的测试用例
- 两个架构行为一致

---

## 其他待办事项

### 内存管理增强
- [ ] 实现 slab 分配器 (替代简单的 kmalloc)
- [ ] 添加内存页回收机制
- [ ] 实现写时复制 (Copy-on-Write)

### 同步原语
- [ ] 实现自旋锁 (spinlock)
- [ ] 实现互斥锁 (mutex)
- [ ] 实现信号量 (semaphore)
- [ ] 实现读写锁 (rwlock)

### 进程管理
- [ ] 实现 fork() 系统调用
- [ ] 实现 exec() 系统调用
- [ ] 实现进程间通信 (IPC)
- [ ] 实现信号机制

### 文件系统
- [ ] 实现可写文件系统 (当前 InitRD 只读)
- [ ] 添加 ext2/ext4 支持
- [ ] 实现 VFS 缓存层

### 网络栈
- [ ] 实现 TCP/IP 协议栈
- [ ] 添加网卡驱动 (virtio-net)
- [ ] 实现 socket API

---

## 开发笔记

### 2025-01-08: 浮点指令问题修复

**问题**: 内核在调度器初始化时触发 Undefined Instruction 异常

**根因**: Makefile.native 缺少 `-mgeneral-regs-only` 编译标志，导致编译器在 task_create() 中生成了浮点指令 (`ldr d0`)

**修复**: 在 ARM64_CFLAGS 中添加 `-mgeneral-regs-only`

**教训**:
1. 裸机内核必须禁止编译器生成浮点/SIMD 指令
2. 使用 `qemu -d int` 和 `objdump -d` 可以快速定位异常指令
3. ESR_EL1 可以区分异常类型 (Undefined Instruction vs Data Abort)

### CPU Context 字段顺序问题

**问题**: cpu_context 结构体字段顺序与汇编代码不匹配

**修复**: 将 `{fp, sp, pc}` 改为 `{fp, pc, sp}`，匹配 `stp x29, x30` 指令

**教训**: C 结构体字段顺序必须与汇编代码的 save/restore 顺序严格一致

---

## 参考资料

- [Linux Kernel CFS Scheduler](https://www.kernel.org/doc/html/latest/scheduler/sched-design-CFS.html)
- [Red-Black Tree Implementation](https://github.com/torvalds/linux/blob/master/lib/rbtree.c)
- [ARM64 Procedure Call Standard](https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst)
- [x86_64 System V ABI](https://gitlab.com/x86-psABIs/x86-64-ABI)
