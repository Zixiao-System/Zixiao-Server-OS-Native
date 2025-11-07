.section .text
.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    lgdt (%rdi)

    mov $0x10, %ax    /* Data segment selector */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* Far return to reload CS */
    pop %rdi
    mov $0x08, %rax   /* Code segment selector */
    push %rax
    push %rdi
    lretq
