.global switch_to
.global arch_setup_task_context

/* void switch_to(task_struct_t* prev, task_struct_t* next) */
/* RDI = prev, RSI = next */
switch_to:
    /* Save prev context (callee-saved registers) */
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    /* Save prev RSP (RDI = prev task) */
    movq %rsp, %rax
    movq %rax, 40(%rdi)      /* task->kernel_sp offset */

    /* Load next RSP (RSI = next task) */
    movq 40(%rsi), %rax
    movq %rax, %rsp

    /* Restore next context */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx

    ret

/* void arch_setup_task_context(task_struct_t* task, void (*entry)(void)) */
/* RDI = task, RSI = entry function */
arch_setup_task_context:
    /* Get task->kernel_sp */
    movq 40(%rdi), %rax      /* RAX = task->kernel_sp */
    subq $48, %rax           /* Reserve space for 6 saved registers */

    /* Set return address to entry function (top of stack after ret) */
    movq %rsi, (%rax)        /* Store entry at bottom of context */

    /* Save adjusted SP back */
    movq %rax, 40(%rdi)
    ret
