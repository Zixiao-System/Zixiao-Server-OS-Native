.section .text.boot

.global _start
.type _start, @function

_start:
    /* Check if we're on CPU 0 */
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, cpu0_boot

    /* Other CPUs: halt */
cpu_halt:
    wfe
    b cpu_halt

cpu0_boot:
    /* Set stack pointer */
    ldr x1, =_stack_top
    mov sp, x1

    /* Clear BSS */
    ldr x1, =__bss_start
    ldr x2, =__bss_end
clear_bss:
    cmp x1, x2
    b.ge bss_cleared
    str xzr, [x1], #8
    b clear_bss

bss_cleared:
    /* Set up exception vector table */
    ldr x0, =exception_vector_table
    msr vbar_el1, x0

    /* Configure system registers for EL1 */

    /* Disable MMU and caches initially */
    mrs x0, sctlr_el1
    bic x0, x0, #(1 << 0)    /* Disable MMU */
    bic x0, x0, #(1 << 2)    /* Disable data cache */
    bic x0, x0, #(1 << 12)   /* Disable instruction cache */
    msr sctlr_el1, x0
    isb

    /* Jump to C kernel */
    bl kernel_main

    /* Halt if kernel returns */
kernel_halt:
    wfi
    b kernel_halt

/* Exception vector table (16 entries, each 128 bytes) */
.align 11
.global exception_vector_table
exception_vector_table:

/* Current EL with SP0 */
.align 7
    b exception_handler    /* Synchronous */
.align 7
    b exception_handler    /* IRQ */
.align 7
    b exception_handler    /* FIQ */
.align 7
    b exception_handler    /* SError */

/* Current EL with SPx */
.align 7
    b exception_handler    /* Synchronous */
.align 7
    b irq_handler          /* IRQ */
.align 7
    b exception_handler    /* FIQ */
.align 7
    b exception_handler    /* SError */

/* Lower EL using AArch64 */
.align 7
    b exception_handler    /* Synchronous */
.align 7
    b exception_handler    /* IRQ */
.align 7
    b exception_handler    /* FIQ */
.align 7
    b exception_handler    /* SError */

/* Lower EL using AArch32 */
.align 7
    b exception_handler    /* Synchronous */
.align 7
    b exception_handler    /* IRQ */
.align 7
    b exception_handler    /* FIQ */
.align 7
    b exception_handler    /* SError */

/* Exception handler stub */
exception_handler:
    /* Save registers */
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    str x30, [sp, #-16]!

    /* Call C exception handler */
    bl arm64_exception_handler

    /* Restore registers */
    ldr x30, [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16

    eret

/* IRQ handler stub */
irq_handler:
    /* Save registers */
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    str x30, [sp, #-16]!

    /* Call C IRQ handler */
    bl arm64_irq_handler

    /* Restore registers */
    ldr x30, [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16

    eret

.size _start, . - _start
