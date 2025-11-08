/**
 * Yuheng Scheduler Core Implementation (MVP)
 */

#include <kernel/sched.h>
#include <kernel/console.h>
#include <kernel/mm.h>
#include <kernel/string.h>

/* Global scheduler state */
static task_struct_t* current_task = NULL;
static task_struct_t* idle_task = NULL;
static task_struct_t* ready_queue = NULL;
static bool scheduler_started = false;  /* Track if scheduler has started */
static uint64_t scheduler_clock = 0;    /* Monotonic tick counter for CFS */
task_struct_t task_table[MAX_TASKS];
uint32_t next_pid = 1;

/* Get current task */
task_struct_t* get_current_task(void) {
    return current_task;
}

/* Get scheduler monotonic time (for CFS vruntime calculations) */
uint64_t scheduler_get_time(void) {
    return scheduler_clock;
}

/******************************************************************************
 * CFS Helper Functions (Preparation Skeletons)
 * ---------------------------------------------
 * These functions will be fully implemented when CFS is enabled.
 * For now, they serve as extension points for future CFS integration.
 *****************************************************************************/

/* Convert priority (0-9) to CFS weight
 * TODO: Implement proper weight table based on nice values
 * Higher priority → higher weight → more CPU time */
static uint32_t priority_to_weight(uint8_t priority) {
    /* Placeholder: return fixed weight for now */
    (void)priority;  /* Unused for now */
    return 1024;  /* Default weight */
}

/* Update current task's execution time and virtual runtime
 * TODO: Implement vruntime calculation: vruntime += delta * (NICE_0_WEIGHT / weight)
 * This is the core of CFS fairness */
static void update_curr_runtime(task_struct_t* task, uint64_t delta) {
    if (!task || task == idle_task) {
        return;
    }

    /* Accumulate actual runtime */
    task->sum_exec_runtime += delta;

    /* TODO: Update vruntime based on weight
     * vruntime += delta * (1024 / task->weight)
     * For now, just increment linearly */
    task->vruntime += delta;
}

/* Check if current task should be preempted by another task
 * TODO: Implement CFS preemption logic based on vruntime delta
 * Preempt if: next->vruntime + threshold < curr->vruntime */
static bool check_preempt_curr(task_struct_t* curr, task_struct_t* next) {
    (void)curr;  /* Unused for now */
    (void)next;  /* Unused for now */

    /* TODO: Implement vruntime-based preemption
     * For now, use time slice (existing RR behavior) */
    return false;
}

/******************************************************************************
 * Runqueue Management
 * -------------------
 * Current implementation: Priority-ordered linked list
 * Future: Replace with red-black tree for CFS O(log n) operations
 *****************************************************************************/

/* Initialize runqueue */
static void runqueue_init(void) {
    ready_queue = NULL;
}

/* Enqueue task (ordered by priority) */
static void runqueue_enqueue(task_struct_t* task) {
    task->state = TASK_READY;

    if (ready_queue == NULL || task->priority > ready_queue->priority) {
        task->next = ready_queue;
        ready_queue = task;
    } else {
        task_struct_t* curr = ready_queue;
        while (curr->next && curr->next->priority >= task->priority) {
            curr = curr->next;
        }
        task->next = curr->next;
        curr->next = task;
    }
}

/* Dequeue task */
static void runqueue_dequeue(task_struct_t* task) {
    if (ready_queue == task) {
        ready_queue = task->next;
    } else {
        task_struct_t* curr = ready_queue;
        while (curr && curr->next != task) {
            curr = curr->next;
        }
        if (curr) {
            curr->next = task->next;
        }
    }
    task->next = NULL;
}

/* Pick next task to run */
static task_struct_t* runqueue_pick_next(void) {
    if (ready_queue) {
        return ready_queue;
    }
    return idle_task;
}

/******************************************************************************
 * Public API for external modules
 *****************************************************************************/

/* Add task to ready queue (public API) */
void task_ready(task_struct_t* task) {
    if (task) {
        runqueue_enqueue(task);
    }
}

/* Pick next task (public API for external use if needed) */
task_struct_t* pick_next_task(void) {
    return runqueue_pick_next();
}

/* Scheduler tick - called every timer interrupt */
void scheduler_tick(void) {
    scheduler_clock++;  /* Increment monotonic scheduler clock */

    if (current_task == NULL || current_task == idle_task) {
        return;
    }

    current_task->total_runtime++;

    if (current_task->policy == SCHED_RR) {
        if (current_task->time_slice > 0) {
            current_task->time_slice--;
        }
    }
}

/* External: arch-specific context switch */
extern void switch_to(task_struct_t* prev, task_struct_t* next);

/******************************************************************************
 * First Task Scheduling
 * ---------------------
 * Special case: First task switch doesn't save previous context
 * (there is no previous task), so we directly restore and jump.
 *****************************************************************************/
static void schedule_first_task(void) {
    task_struct_t* next = runqueue_pick_next();
    if (!next || next == idle_task) {
        /* Edge case: No tasks ready. Just mark scheduler as started and return.
         * Idle task will run on next schedule() call. */
        scheduler_started = true;
        return;
    }

    scheduler_started = true;
    next->state = TASK_RUNNING;
    next->switches++;
    current_task = next;
    runqueue_dequeue(next);

    console_printf("[Scheduler] First switch to task %s (PID=%u)\n",
                   next->name, next->pid);

    /* Restore registers from next->cpu_context and jump to task */
    __asm__ volatile(
        "mov x8, %0\n"               /* x8 = &next->cpu_context */
        "ldp x19, x20, [x8], #16\n"
        "ldp x21, x22, [x8], #16\n"
        "ldp x23, x24, [x8], #16\n"
        "ldp x25, x26, [x8], #16\n"
        "ldp x27, x28, [x8], #16\n"
        "ldp x29, x30, [x8], #16\n"  /* Restore FP and LR */
        "ldr x9, [x8]\n"             /* Load SP */
        "mov sp, x9\n"
        "ret\n"                      /* Jump to task entry (in x30/LR) */
        :: "r"(&next->cpu_context)
        : "memory"
    );
    __builtin_unreachable();
}

/* Schedule: select and switch to next task */
void schedule(void) {
    if (current_task == NULL) {
        return;
    }

    /* First time scheduling: jump directly to first task without saving context */
    if (!scheduler_started) {
        schedule_first_task();
        /* NOTREACHED */
    }

    task_struct_t* prev = current_task;
    task_struct_t* next = runqueue_pick_next();

    if (prev == next) {
        return;
    }

    /* Save prev state if still ready */
    if (prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
        if (prev != idle_task && prev->time_slice > 0) {
            runqueue_enqueue(prev);
        } else if (prev != idle_task) {
            prev->time_slice = DEFAULT_TIMESLICE;
            runqueue_enqueue(prev);
        }
    }

    /* Dequeue next from ready queue */
    if (next != idle_task) {
        runqueue_dequeue(next);
    }
    next->state = TASK_RUNNING;
    next->switches++;

    current_task = next;

    /* Perform context switch */
    switch_to(prev, next);
}

/* Task yield */
void task_yield(void) {
    schedule();
}

/* Initialize scheduler */
void scheduler_init(void) {
    console_printf("  [*] Initializing Yuheng scheduler...\n");

    memset(task_table, 0, sizeof(task_table));

    next_pid = 1;
    runqueue_init();

    /* Create idle task - will be implemented in idle.c */
    extern void idle_task_entry(void);
    idle_task = task_create("idle", idle_task_entry, 0, 4096);
    idle_task->pid = 0;
    current_task = idle_task;

    console_printf("      Yuheng scheduler initialized\n");
}
