.global switch_to
.global arch_setup_task_context

/*
 * void switch_to(task_struct_t* prev, task_struct_t* next)
 * x0 = prev task
 * x1 = next task
 *
 * CPU context offset in task_struct:
 * - pid (4) + name (16) + [2 pad] + priority (1) + policy (1) + state (4) +
 *   time_slice (4) + total_runtime (8) + vruntime (8) + exec_start (8) +
 *   weight (4) + [4 pad] + sum_exec_runtime (8) = 72
 */
.equ CPU_CONTEXT_OFFSET, 72

switch_to:
    /* Calculate address of prev->cpu_context */
    add     x8, x0, #CPU_CONTEXT_OFFSET

    /* Save prev task's registers to prev->cpu_context */
    stp     x19, x20, [x8], #16
    stp     x21, x22, [x8], #16
    stp     x23, x24, [x8], #16
    stp     x25, x26, [x8], #16
    stp     x27, x28, [x8], #16
    stp     x29, x30, [x8], #16      /* Save FP and LR */
    mov     x9, sp
    str     x9, [x8]                 /* Save SP */

    /* Calculate address of next->cpu_context */
    add     x8, x1, #CPU_CONTEXT_OFFSET

    /* Restore next task's registers from next->cpu_context */
    ldp     x19, x20, [x8], #16
    ldp     x21, x22, [x8], #16
    ldp     x23, x24, [x8], #16
    ldp     x25, x26, [x8], #16
    ldp     x27, x28, [x8], #16
    ldp     x29, x30, [x8], #16      /* Restore FP and LR */
    ldr     x9, [x8]                 /* Restore SP */
    mov     sp, x9

    ret                              /* Return to next task (jumps to LR) */

/* void arch_setup_task_context(task_struct_t* task, void (*entry)(void)) */
arch_setup_task_context:
    /* x0 = task, x1 = entry function */

    /* Get stack pointer from task->kernel_stack + task->kernel_stack_size */
    ldr     x2, [x0, #CPU_CONTEXT_OFFSET + 104]     /* task->kernel_stack offset */
    ldr     w3, [x0, #CPU_CONTEXT_OFFSET + 112]     /* task->kernel_stack_size offset */
    add     x2, x2, x3                               /* x2 = stack top */

    /* Align SP to 16 bytes */
    and     x2, x2, #-16

    /* Calculate address of task->cpu_context */
    add     x8, x0, #CPU_CONTEXT_OFFSET

    /* Initialize cpu_context */
    stp     xzr, xzr, [x8], #16       /* x19, x20 = 0 */
    stp     xzr, xzr, [x8], #16       /* x21, x22 = 0 */
    stp     xzr, xzr, [x8], #16       /* x23, x24 = 0 */
    stp     xzr, xzr, [x8], #16       /* x25, x26 = 0 */
    stp     xzr, xzr, [x8], #16       /* x27, x28 = 0 */
    stp     xzr, x1,  [x8], #16       /* fp = 0, pc = entry function (x30) */
    str     x2, [x8]                  /* sp = stack top */

    ret
