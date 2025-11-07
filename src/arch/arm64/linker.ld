/* ARM64 Linker Script for Zixiao OS */
ENTRY(_start)

SECTIONS
{
    /* QEMU virt machine loads kernel at 0x40000000 */
    . = 0x40000000;

    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    /* Stack (grows downward) */
    . = ALIGN(16);
    . += 0x4000; /* 16KB stack */
    _stack_top = .;

    _kernel_end = .;
}
