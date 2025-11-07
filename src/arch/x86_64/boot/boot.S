.section .multiboot
.align 8

/* Multiboot2 Header */
multiboot_header_start:
    .long 0xe85250d6                   /* Magic number */
    .long 0                             /* Architecture: i386 */
    .long multiboot_header_end - multiboot_header_start  /* Header length */
    .long -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))  /* Checksum */

    /* End tag */
    .short 0    /* type */
    .short 0    /* flags */
    .long 8     /* size */
multiboot_header_end:

/* Bootstrap stack */
.section .bss
.align 16
stack_bottom:
    .skip 16384  /* 16 KiB stack */
stack_top:

/* Page tables for long mode */
.align 4096
boot_pml4:
    .skip 4096
boot_pdpt:
    .skip 4096
boot_pd:
    .skip 4096

.section .text
.global _start
.type _start, @function

_start:
    /* Disable interrupts */
    cli

    /* Set up stack */
    mov $stack_top, %esp

    /* Save Multiboot info pointer */
    push %ebx
    push %eax

    /* Check for CPUID support */
    pushfl
    pop %eax
    mov %eax, %ecx
    xor $(1 << 21), %eax
    push %eax
    popfl
    pushfl
    pop %eax
    push %ecx
    popfl
    cmp %ecx, %eax
    je no_long_mode

    /* Check for long mode support */
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $(1 << 29), %edx    /* Check LM bit */
    jz no_long_mode

    /* Set up page tables for identity mapping */
    /* PML4[0] -> PDPT */
    mov $boot_pdpt, %eax
    or $0x3, %eax             /* Present + Writable */
    mov %eax, boot_pml4

    /* PDPT[0] -> PD */
    mov $boot_pd, %eax
    or $0x3, %eax
    mov %eax, boot_pdpt

    /* Identity map first 2MB using 2MB pages */
    mov $0x83, %eax           /* Present + Writable + Huge page */
    mov %eax, boot_pd

    /* Load PML4 */
    mov $boot_pml4, %eax
    mov %eax, %cr3

    /* Enable PAE */
    mov %cr4, %eax
    or $(1 << 5), %eax
    mov %eax, %cr4

    /* Enable long mode */
    mov $0xC0000080, %ecx
    rdmsr
    or $(1 << 8), %eax
    wrmsr

    /* Enable paging */
    mov %cr0, %eax
    or $(1 << 31), %eax
    mov %eax, %cr0

    /* Load GDT */
    lgdt gdt64_pointer

    /* Far jump to 64-bit code */
    ljmp $0x08, $long_mode_start

no_long_mode:
    /* Print "E" to screen (Error) */
    mov $0xb8000, %edi
    movw $0x4f45, (%edi)
    hlt
    jmp no_long_mode

.section .text
.code64
long_mode_start:
    /* Clear segment registers */
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Set up 64-bit stack */
    mov $stack_top, %rsp

    /* Restore multiboot info */
    pop %rdi   /* Magic */
    pop %rsi   /* MBI pointer */

    /* Call C kernel */
    call kernel_main

    /* Halt if kernel returns */
halt:
    cli
    hlt
    jmp halt

/* GDT for long mode */
.section .rodata
.align 16
gdt64:
    .quad 0                                /* Null descriptor */
    .quad 0x00209A0000000000              /* Code segment (executable, 64-bit) */
    .quad 0x0000920000000000              /* Data segment */

gdt64_pointer:
    .word gdt64_pointer - gdt64 - 1
    .quad gdt64
