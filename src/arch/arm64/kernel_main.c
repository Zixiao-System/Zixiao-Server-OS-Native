#include <kernel/console.h>
#include <kernel/types.h>
#include <arch/interrupts.h>

extern char uart_getchar(void);
extern bool uart_has_data(void);

void kernel_main(void) {
    /* Initialize console (UART) */
    console_init();

    /* Print welcome message */
    console_printf("========================================\n");
    console_printf("  Zixiao Server OS - ARM64 Edition\n");
    console_printf("========================================\n\n");

    console_printf("Kernel loaded successfully!\n");
    console_printf("Architecture: ARM64 (AArch64)\n");
    console_printf("Running at EL1 (Kernel mode)\n\n");

    console_printf("Initializing subsystems...\n");

    /* Initialize interrupts */
    console_printf("  [*] Initializing exception handlers...\n");
    interrupts_init();

    /* Enable interrupts */
    console_printf("  [*] Enabling interrupts...\n");
    interrupts_enable();

    console_printf("\nAll systems initialized!\n\n");
    console_printf("UART console ready. Type something:\n");
    console_printf("(Press Ctrl+A then X to exit QEMU)\n\n");

    /* Simple echo loop */
    while (1) {
        if (uart_has_data()) {
            char c = uart_getchar();
            console_putchar(c);  /* Echo back */
        }
        __asm__ volatile("wfi");  /* Wait for interrupt */
    }
}
