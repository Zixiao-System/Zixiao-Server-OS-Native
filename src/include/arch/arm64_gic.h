/**
 * ARM64 Generic Interrupt Controller (GICv2) Driver
 * For QEMU virt machine
 */

#ifndef ARM64_GIC_H
#define ARM64_GIC_H

#include <kernel/types.h>

/* GICv2 Memory-Mapped Addresses (QEMU virt machine) */
#define GICD_BASE       0x08000000  /* Distributor base */
#define GICC_BASE       0x08010000  /* CPU Interface base */

/* GIC Distributor registers (GICD) */
#define GICD_CTLR       (GICD_BASE + 0x000)   /* Control Register */
#define GICD_TYPER      (GICD_BASE + 0x004)   /* Type Register */
#define GICD_ISENABLER  (GICD_BASE + 0x100)   /* Interrupt Set-Enable Registers */
#define GICD_ICENABLER  (GICD_BASE + 0x180)   /* Interrupt Clear-Enable Registers */
#define GICD_ISPENDR    (GICD_BASE + 0x200)   /* Interrupt Set-Pending Registers */
#define GICD_ICPENDR    (GICD_BASE + 0x280)   /* Interrupt Clear-Pending Registers */
#define GICD_IPRIORITYR (GICD_BASE + 0x400)   /* Interrupt Priority Registers */
#define GICD_ITARGETSR  (GICD_BASE + 0x800)   /* Interrupt Processor Targets Registers */
#define GICD_ICFGR      (GICD_BASE + 0xC00)   /* Interrupt Configuration Registers */

/* GIC CPU Interface registers (GICC) */
#define GICC_CTLR       (GICC_BASE + 0x000)   /* CPU Interface Control Register */
#define GICC_PMR        (GICC_BASE + 0x004)   /* Interrupt Priority Mask Register */
#define GICC_IAR        (GICC_BASE + 0x00C)   /* Interrupt Acknowledge Register */
#define GICC_EOIR       (GICC_BASE + 0x010)   /* End of Interrupt Register */

/* GIC Constants */
#define GIC_MAX_IRQS    1024
#define GIC_SGI_MAX     16      /* Software-Generated Interrupts (0-15) */
#define GIC_PPI_MAX     32      /* Private Peripheral Interrupts (16-31) */
#define GIC_SPI_BASE    32      /* Shared Peripheral Interrupts start at 32 */

/* ARM Generic Timer IRQ numbers (PPIs on QEMU virt) */
#define IRQ_TIMER_PHYS  30      /* Physical Timer (PPI 14, IRQ 30) */
#define IRQ_TIMER_VIRT  27      /* Virtual Timer (PPI 11, IRQ 27) */

/* IRQ handler function pointer type */
typedef void (*irq_handler_t)(void);

/**
 * Initialize GIC (Distributor and CPU Interface)
 */
void gic_init(void);

/**
 * Enable a specific IRQ
 * @param irq - IRQ number (0-1023)
 */
void gic_enable_irq(uint32_t irq);

/**
 * Disable a specific IRQ
 * @param irq - IRQ number (0-1023)
 */
void gic_disable_irq(uint32_t irq);

/**
 * Set IRQ priority
 * @param irq - IRQ number
 * @param priority - Priority value (0-255, lower = higher priority)
 */
void gic_set_priority(uint32_t irq, uint8_t priority);

/**
 * Acknowledge an interrupt and return its IRQ number
 * @return IRQ number (0-1023) or 1023 if spurious
 */
uint32_t gic_acknowledge_irq(void);

/**
 * Signal end of interrupt handling
 * @param irq - IRQ number to signal completion
 */
void gic_end_of_interrupt(uint32_t irq);

/**
 * Install an IRQ handler
 * @param irq - IRQ number
 * @param handler - Handler function pointer
 */
void gic_install_handler(uint32_t irq, irq_handler_t handler);

/**
 * Uninstall an IRQ handler
 * @param irq - IRQ number
 */
void gic_uninstall_handler(uint32_t irq);

/**
 * Main IRQ dispatch routine (called from assembly IRQ handler)
 */
void gic_handle_irq(void);

#endif /* ARM64_GIC_H */
