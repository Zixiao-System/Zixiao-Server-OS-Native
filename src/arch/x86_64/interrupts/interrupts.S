.section .text

/* Macro to create ISR stubs (no error code) */
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    push $0              /* Dummy error code */
    push $\num
    jmp isr_common_stub
.endm

/* Macro to create ISR stubs (with error code) */
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    push $\num
    jmp isr_common_stub
.endm

/* Macro to create IRQ stubs */
.macro IRQ num, irq_num
.global irq\num
irq\num:
    push $0
    push $\irq_num
    jmp irq_common_stub
.endm

/* CPU exception ISRs (0-31) */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE 30
ISR_NOERRCODE 31

/* IRQs (32-47) */
IRQ 32, 0
IRQ 33, 1
IRQ 34, 2
IRQ 35, 3
IRQ 36, 4
IRQ 37, 5
IRQ 38, 6
IRQ 39, 7
IRQ 40, 8
IRQ 41, 9
IRQ 42, 10
IRQ 43, 11
IRQ 44, 12
IRQ 45, 13
IRQ 46, 14
IRQ 47, 15

/* Remaining ISRs (48-255) */
.set i, 48
.rept 208
    ISR_NOERRCODE i
    .set i, i+1
.endr

/* Common ISR handler */
extern isr_handler
isr_common_stub:
    /* Save all registers */
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    /* Call C handler */
    mov 120(%rsp), %rdi  /* Get interrupt number */
    call isr_handler

    /* Restore registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    /* Clean up error code and ISR number */
    add $16, %rsp
    iretq

/* Common IRQ handler */
extern irq_handler
irq_common_stub:
    /* Save all registers */
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    /* Call C handler */
    mov 120(%rsp), %rdi  /* Get IRQ number */
    call irq_handler

    /* Restore registers */
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    /* Clean up error code and IRQ number */
    add $16, %rsp
    iretq

/* IDT load function */
.global idt_load
.type idt_load, @function
idt_load:
    lidt (%rdi)
    ret

/* ISR stub table for setting up IDT */
.section .rodata
.global isr_stub_table
isr_stub_table:
.set i, 0
.rept 32
    .quad isr\i
    .set i, i+1
.endr
.set i, 32
.rept 16
    .quad irq\i
    .set i, i+1
.endr
.set i, 48
.rept 208
    .quad isr\i
    .set i, i+1
.endr
