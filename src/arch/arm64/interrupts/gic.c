/**
 * ARM64 Generic Interrupt Controller (GICv2) Driver
 */

#include <arch/arm64_gic.h>
#include <kernel/console.h>
#include <kernel/string.h>

/* IRQ handler table */
static irq_handler_t irq_handlers[GIC_MAX_IRQS];

/* Helper: Read 32-bit MMIO register */
static inline uint32_t mmio_read32(uint64_t addr) {
    return *(volatile uint32_t*)addr;
}

/* Helper: Write 32-bit MMIO register */
static inline void mmio_write32(uint64_t addr, uint32_t value) {
    *(volatile uint32_t*)addr = value;
}

/**
 * Initialize GIC
 */
void gic_init(void) {
    console_printf("      Initializing GICv2...\n");

    /* Clear IRQ handler table */
    memset(irq_handlers, 0, sizeof(irq_handlers));

    /* 1. Disable distributor */
    mmio_write32(GICD_CTLR, 0);

    /* 2. Get number of interrupt lines supported */
    uint32_t typer = mmio_read32(GICD_TYPER);
    uint32_t num_irqs = ((typer & 0x1F) + 1) * 32;
    console_printf("      GIC: %u IRQ lines\n", num_irqs);

    /* 3. Disable all interrupts */
    for (uint32_t i = 0; i < num_irqs; i += 32) {
        mmio_write32(GICD_ICENABLER + (i / 32) * 4, 0xFFFFFFFF);
    }

    /* 4. Clear all pending interrupts */
    for (uint32_t i = 0; i < num_irqs; i += 32) {
        mmio_write32(GICD_ICPENDR + (i / 32) * 4, 0xFFFFFFFF);
    }

    /* 5. Set all interrupts to lowest priority */
    for (uint32_t i = 0; i < num_irqs; i += 4) {
        mmio_write32(GICD_IPRIORITYR + i, 0xA0A0A0A0);  /* Priority 0xA0 */
    }

    /* 6. Route all SPIs to CPU 0 (target CPU interface 0) */
    for (uint32_t i = GIC_SPI_BASE; i < num_irqs; i += 4) {
        mmio_write32(GICD_ITARGETSR + i, 0x01010101);  /* CPU 0 */
    }

    /* 7. Set all interrupts as level-sensitive, active-high (default) */
    for (uint32_t i = 0; i < num_irqs; i += 16) {
        mmio_write32(GICD_ICFGR + (i / 16) * 4, 0x00000000);
    }

    /* 8. Enable distributor */
    mmio_write32(GICD_CTLR, 1);

    /* 9. Configure CPU Interface */
    /* Set priority mask to lowest priority (allow all interrupts) */
    mmio_write32(GICC_PMR, 0xFF);

    /* Enable CPU interface */
    mmio_write32(GICC_CTLR, 1);

    console_printf("      GIC initialized\n");
}

/**
 * Enable a specific IRQ
 */
void gic_enable_irq(uint32_t irq) {
    if (irq >= GIC_MAX_IRQS) return;

    uint32_t reg = GICD_ISENABLER + (irq / 32) * 4;
    uint32_t bit = 1 << (irq % 32);
    mmio_write32(reg, bit);
}

/**
 * Disable a specific IRQ
 */
void gic_disable_irq(uint32_t irq) {
    if (irq >= GIC_MAX_IRQS) return;

    uint32_t reg = GICD_ICENABLER + (irq / 32) * 4;
    uint32_t bit = 1 << (irq % 32);
    mmio_write32(reg, bit);
}

/**
 * Set IRQ priority
 */
void gic_set_priority(uint32_t irq, uint8_t priority) {
    if (irq >= GIC_MAX_IRQS) return;

    uint32_t reg = GICD_IPRIORITYR + irq;
    mmio_write32(reg, priority);
}

/**
 * Acknowledge an interrupt
 */
uint32_t gic_acknowledge_irq(void) {
    uint32_t iar = mmio_read32(GICC_IAR);
    return iar & 0x3FF;  /* IRQ number is in bits [9:0] */
}

/**
 * Signal end of interrupt
 */
void gic_end_of_interrupt(uint32_t irq) {
    mmio_write32(GICC_EOIR, irq);
}

/**
 * Install an IRQ handler
 */
void gic_install_handler(uint32_t irq, irq_handler_t handler) {
    if (irq >= GIC_MAX_IRQS) return;
    irq_handlers[irq] = handler;
}

/**
 * Uninstall an IRQ handler
 */
void gic_uninstall_handler(uint32_t irq) {
    if (irq >= GIC_MAX_IRQS) return;
    irq_handlers[irq] = NULL;
}

/**
 * Main IRQ dispatch routine
 */
void gic_handle_irq(void) {
    /* Acknowledge interrupt and get IRQ number */
    uint32_t irq = gic_acknowledge_irq();

    /* Check for spurious interrupt */
    if (irq >= 1020) {
        /* Spurious interrupt, ignore */
        return;
    }

    /* Call registered handler if exists */
    if (irq < GIC_MAX_IRQS && irq_handlers[irq] != NULL) {
        irq_handlers[irq]();
    } else {
        console_printf("[GIC] Unhandled IRQ %u\n", irq);
    }

    /* Signal end of interrupt */
    gic_end_of_interrupt(irq);
}
