#ifndef ZIXIAO_INTERRUPTS_H
#define ZIXIAO_INTERRUPTS_H

#include <kernel/types.h>

// Architecture-specific interrupt initialization
void interrupts_init(void);

// Enable/disable interrupts
void interrupts_enable(void);
void interrupts_disable(void);

// IRQ handlers
typedef void (*irq_handler_t)(void);
void irq_install_handler(uint8_t irq, irq_handler_t handler);
void irq_uninstall_handler(uint8_t irq);

#endif // ZIXIAO_INTERRUPTS_H