#include <kernel/console.h>
#include <kernel/types.h>

void arm64_exception_handler(void) {
    console_printf("\n*** ARM64 EXCEPTION ***\n");
    console_printf("System halted.\n");
    while (1) {
        __asm__ volatile("wfi");
    }
}

void arm64_irq_handler(void) {
    /* IRQ handling - to be implemented with GIC */
    console_printf("IRQ received\n");
}

void interrupts_init(void) {
    /* GIC initialization - placeholder for now */
    console_printf("ARM64 interrupt system initialized\n");
}

void interrupts_enable(void) {
    __asm__ volatile("msr daifclr, #2" ::: "memory");
}

void interrupts_disable(void) {
    __asm__ volatile("msr daifset, #2" ::: "memory");
}

void irq_install_handler(uint8_t irq, void (*handler)(void)) {
    /* GIC IRQ handler installation - placeholder */
    (void)irq;
    (void)handler;
}

void irq_uninstall_handler(uint8_t irq) {
    /* GIC IRQ handler removal - placeholder */
    (void)irq;
}
