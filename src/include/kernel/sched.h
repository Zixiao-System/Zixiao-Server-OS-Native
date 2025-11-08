/**
 * Yuheng Scheduler - Core Data Structures and API
 * 玉衡调度器 - 实时任务调度系统
 */

#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include <kernel/types.h>

/* Task states */
typedef enum {
    TASK_RUNNING,    /* Currently executing */
    TASK_READY,      /* Ready to run */
    TASK_BLOCKED,    /* Waiting for resource */
    TASK_SLEEPING,   /* Sleeping until timer */
    TASK_ZOMBIE      /* Exited, waiting for cleanup */
} task_state_t;

/* Scheduling policies */
typedef enum {
    SCHED_FIFO = 0,     /* Real-time FIFO */
    SCHED_RR = 1,       /* Real-time Round-Robin */
    SCHED_NORMAL = 2    /* Normal priority-based */
} sched_policy_t;


/*
 * 玉衡调度器上下文切换笔记
 * --------------------------
 * 2025-06-12: Claude AI 为了躲 GPL，把 cpu_context 临时压在栈上，
 *             导致第二次切换 pop 出 0xdeadbeef，全场熄火。
 *             提醒我们：GPL 可以躲，bug 躲不掉。
 * 2025-06-13: 把保存区挪回 struct task，从此“星”归位，玉衡长明。
 */

/* CPU context saved during context switch */
typedef struct cpu_context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;    /* x29 */
    uint64_t pc;    /* x30/LR - program counter */
    uint64_t sp;    /* Saved stack pointer */
} cpu_context_t;

/* Task structure (Process Control Block) */
typedef struct task_struct {
    /* Identity */
    uint32_t pid;
    char name[16];

    /* Scheduling */
    uint8_t priority;           /* 0-9 (simplified for MVP) */
    uint8_t policy;             /* sched_policy_t */
    task_state_t state;
    uint32_t time_slice;        /* Ticks remaining (RR/FIFO) */
    uint64_t total_runtime;     /* Total ticks executed */

    /* CFS scheduling fields */
    uint64_t vruntime;          /* Virtual runtime (CFS fairness metric) */
    uint64_t exec_start;        /* When task started running (for delta calc) */
    uint32_t weight;            /* Priority weight (higher priority = higher weight) */
    uint64_t sum_exec_runtime;  /* Cumulative execution time (debugging) */

    /* Context - saved registers */
    cpu_context_t cpu_context;  /* Saved CPU state */

    /* Stack */
    void* kernel_stack;         /* Stack base address */
    uint32_t kernel_stack_size; /* Stack size in bytes */

    /* List linkage */
    struct task_struct* next;
    struct task_struct* prev;

    /* Statistics */
    uint64_t switches;          /* Context switch count */
} task_struct_t;

/* Maximum number of tasks */
#define MAX_TASKS 256

/* Default time slice (10 ticks = 100ms at 100Hz) */
#define DEFAULT_TIMESLICE 10

/* Scheduler API */
void scheduler_init(void);
void scheduler_tick(void);
void schedule(void);
task_struct_t* get_current_task(void);
void task_ready(task_struct_t* task);  /* Add task to ready queue */

/* Scheduler time source (for CFS vruntime calculations) */
uint64_t scheduler_get_time(void);

/* Task creation/destruction */
task_struct_t* task_create(const char* name, void (*entry)(void),
                           uint8_t priority, uint32_t stack_size);
void task_exit(void);
void task_yield(void);

/* Internal: Pick next task to run */
task_struct_t* pick_next_task(void);

#endif /* KERNEL_SCHED_H */
