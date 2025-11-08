/**
 * x86_64 PIT (Programmable Interval Timer - 8254) Driver
 */

#include <arch/x86_64_timer.h>
#include <arch/interrupts.h>
#include <kernel/console.h>
#include <kernel/sched.h>

/* Timer state */
static uint64_t timer_ticks = 0;

/* Helper: Write byte to I/O port */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Helper: Read byte from I/O port */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * Timer interrupt handler
 */
void timer_irq_handler(void) {
    /* Increment tick counter */
    timer_ticks++;

    /* Call scheduler tick to update time slices */
    scheduler_tick();

    /* Print tick every second (for debugging) */
    if (timer_ticks % TIMER_TICK_HZ == 0) {
        console_printf("[Timer] Tick %llu (uptime: %llu seconds)\n",
                       timer_ticks, timer_ticks / TIMER_TICK_HZ);
    }

    /* Trigger task switch every 10 ticks (100ms) */
    if (timer_ticks % 10 == 0) {
        schedule();
    }

    /* Note: PIC EOI (End of Interrupt) is sent automatically by irq_handler() in idt.c */
}

/**
 * Initialize PIT timer
 */
void timer_init(void) {
    console_printf("      Initializing Intel 8254 PIT...\n");

    /* Calculate divisor for desired frequency */
    uint32_t divisor = PIT_BASE_FREQ / TIMER_TICK_HZ;
    console_printf("      PIT base frequency: %u Hz\n", PIT_BASE_FREQ);
    console_printf("      PIT divisor: %u (%u Hz)\n", divisor, TIMER_TICK_HZ);

    /* Reset tick counter */
    timer_ticks = 0;

    /* Configure PIT:
     * - Channel 0 (system timer)
     * - Mode 2 (rate generator - auto-reload)
     * - Access mode: low byte, then high byte
     * - Binary counter (not BCD)
     */
    uint8_t command = PIT_CMD_CHANNEL0 | PIT_CMD_LOHI | PIT_CMD_MODE2 | PIT_CMD_BINARY;
    outb(PIT_COMMAND, command);

    /* Write divisor (low byte, then high byte) */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    /* Install timer IRQ handler */
    console_printf("      Installing IRQ 0 handler...\n");
    irq_install_handler(0, timer_irq_handler);

    /* PIC IRQ 0 will be unmasked in idt.c pic_remap() */

    console_printf("      PIT initialized and enabled\n");
}

/**
 * Get timer frequency
 */
uint64_t timer_get_frequency(void) {
    return PIT_BASE_FREQ;
}

/**
 * Get tick count
 */
uint64_t timer_get_ticks(void) {
    return timer_ticks;
}

/**
 * Get uptime in milliseconds
 */
uint64_t timer_get_uptime_ms(void) {
    return (timer_ticks * 1000) / TIMER_TICK_HZ;
}

/**
 * Sleep for specified milliseconds (busy wait)
 *
 * Note: This is a simple busy-wait implementation.
 * For more efficient sleep, wait for scheduler implementation.
 */
void timer_sleep_ms(uint32_t ms) {
    uint64_t target_ticks = timer_ticks + ((uint64_t)ms * TIMER_TICK_HZ) / 1000;

    /* Busy wait until target ticks reached */
    while (timer_ticks < target_ticks) {
        __asm__ volatile("pause");  /* x86 PAUSE instruction for spin-wait loop */
    }
}
