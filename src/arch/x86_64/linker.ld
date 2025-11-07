/* x86_64 Linker Script for Zixiao OS */
ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 1MB physical address */
    . = 1M;

    .boot :
    {
        *(.multiboot)
    }

    /* Align to page boundary */
    . = ALIGN(4K);

    .text : ALIGN(4K)
    {
        *(.text*)
    }

    .rodata : ALIGN(4K)
    {
        *(.rodata*)
    }

    .data : ALIGN(4K)
    {
        *(.data*)
    }

    .bss : ALIGN(4K)
    {
        *(COMMON)
        *(.bss*)
    }

    /* End of kernel */
    . = ALIGN(4K);
    _kernel_end = .;
}
