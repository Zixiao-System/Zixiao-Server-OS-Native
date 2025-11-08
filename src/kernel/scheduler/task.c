/**
 * Task Management - Creation and Destruction
 */

#include <kernel/sched.h>
#include <kernel/console.h>
#include <kernel/mm.h>
#include <kernel/string.h>

extern task_struct_t task_table[MAX_TASKS];
extern uint32_t next_pid;

/* Architecture-specific: setup initial task context */
extern void arch_setup_task_context(task_struct_t* task, void (*entry)(void));

/* Create new task */
task_struct_t* task_create(const char* name, void (*entry)(void),
                           uint8_t priority, uint32_t stack_size) {
    /* Find free PID */
    if (next_pid >= MAX_TASKS) {
        console_printf("ERROR: MAX_TASKS reached\n");
        return NULL;
    }

    task_struct_t* task = &task_table[next_pid++];
    memset(task, 0, sizeof(task_struct_t));

    /* Initialize task */
    task->pid = next_pid - 1;

    /* Copy name manually (avoid strncpy) */
    int i;
    for (i = 0; i < 15 && name[i] != '\0'; i++) {
        task->name[i] = name[i];
    }
    task->name[i] = '\0';

    task->priority = priority;
    task->policy = SCHED_RR;
    task->state = TASK_READY;
    task->time_slice = DEFAULT_TIMESLICE;

    /* Initialize CFS fields */
    task->vruntime = 0;
    task->exec_start = 0;
    task->weight = 1024;  /* Default weight (normal priority) */
    task->sum_exec_runtime = 0;

    /* Allocate kernel stack */
    task->kernel_stack_size = stack_size;
    task->kernel_stack = kmalloc(stack_size);
    if (!task->kernel_stack) {
        console_printf("ERROR: Failed to allocate stack for task %s\n", name);
        return NULL;
    }

    /* Setup initial context (arch-specific) */
    arch_setup_task_context(task, entry);

    console_printf("      Task %s created (PID=%u, priority=%u)\n",
                   task->name, task->pid, task->priority);

    return task;
}

/* Exit current task */
void task_exit(void) {
    task_struct_t* task = get_current_task();
    console_printf("[Task %s] Exiting\n", task->name);

    task->state = TASK_ZOMBIE;
    kfree(task->kernel_stack);

    /* Trigger reschedule - will never return */
    schedule();
}
