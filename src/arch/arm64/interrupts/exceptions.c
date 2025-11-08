#include <kernel/console.h>
#include <kernel/types.h>
#include <arch/arm64_gic.h>

void arm64_exception_handler(void) {
    console_printf("\n*** ARM64 EXCEPTION ***\n");
    console_printf("System halted.\n");
    while (1) {
        __asm__ volatile("wfi");
    }
}

void arm64_irq_handler(void) {
    /* Dispatch IRQ via GIC */
    gic_handle_irq();
}

void interrupts_init(void) {
    /* Initialize GIC */
    gic_init();
}

void interrupts_enable(void) {
    /* Unmask IRQ interrupts at CPU level (clear DAIF.I bit) */
    __asm__ volatile("msr daifclr, #2" ::: "memory");
}

void interrupts_disable(void) {
    /* Mask IRQ interrupts at CPU level (set DAIF.I bit) */
    __asm__ volatile("msr daifset, #2" ::: "memory");
}
