#include <kernel/console.h>
#include <kernel/types.h>
#include <arch/interrupts.h>

/* External initialization functions */
extern void gdt_init(void);
extern void keyboard_init(void);

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    /* Initialize console */
    console_init();

    /* Print welcome message */
    console_printf("========================================\n");
    console_printf("  Zixiao Server OS - x86_64 Edition\n");
    console_printf("========================================\n\n");

    console_printf("Kernel loaded successfully!\n");
    console_printf("Multiboot2 magic: 0x%x\n", magic);
    console_printf("MBI address: 0x%x\n\n", mbi_addr);

    console_printf("Architecture: x86_64 (Long Mode)\n");
    console_printf("Status: Running in 64-bit mode\n\n");

    console_printf("Initializing subsystems...\n");

    /* Initialize GDT */
    console_printf("  [*] Initializing GDT...\n");
    gdt_init();

    /* Initialize interrupts */
    console_printf("  [*] Initializing IDT...\n");
    interrupts_init();

    /* Initialize keyboard driver */
    console_printf("  [*] Initializing keyboard driver...\n");
    keyboard_init();

    /* Enable interrupts */
    console_printf("  [*] Enabling interrupts...\n");
    interrupts_enable();

    console_printf("\nAll systems initialized!\n\n");
    console_printf("Type something on the keyboard...\n");
    console_printf("(Press Ctrl+C in terminal to exit QEMU)\n\n");

    /* Idle loop */
    while (1) {
        __asm__ volatile("hlt");
    }
}
