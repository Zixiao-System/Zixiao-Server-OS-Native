/**
 * ARM64 Generic Timer Driver
 */

#include <arch/arm64_timer.h>
#include <arch/arm64_gic.h>
#include <kernel/console.h>
#include <kernel/sched.h>

/* Timer state */
static uint64_t timer_frequency = 0;
static uint64_t timer_ticks = 0;
static uint64_t timer_interval = 0;  /* Counter ticks per timer tick */

/**
 * Read timer counter (CNTPCT_EL0)
 */
uint64_t timer_get_counter(void) {
    uint64_t count;
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(count));
    return count;
}

/**
 * Read timer frequency (CNTFRQ_EL0)
 */
static uint64_t timer_read_frequency(void) {
    uint64_t freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    return freq;
}

/**
 * Write timer compare value (CNTP_CVAL_EL0)
 */
static void timer_write_cval(uint64_t value) {
    __asm__ volatile("msr cntp_cval_el0, %0" :: "r"(value));
    __asm__ volatile("isb");
}

/**
 * Write timer control register (CNTP_CTL_EL0)
 * Bit 0: ENABLE - Enable timer
 * Bit 1: IMASK - Interrupt mask (0 = not masked)
 * Bit 2: ISTATUS - Timer condition met (read-only)
 */
static void timer_write_control(uint32_t value) {
    __asm__ volatile("msr cntp_ctl_el0, %0" :: "r"((uint64_t)value));
    __asm__ volatile("isb");
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

    /* Set next timer interrupt */
    uint64_t current = timer_get_counter();
    timer_write_cval(current + timer_interval);
}

/**
 * Initialize timer
 */
void timer_init(void) {
    console_printf("      Initializing ARM Generic Timer...\n");

    /* Read timer frequency from system register */
    timer_frequency = timer_read_frequency();
    console_printf("      Timer frequency: %llu Hz\n", timer_frequency);

    /* Calculate interval for desired tick rate */
    timer_interval = timer_frequency / TIMER_TICK_HZ;
    console_printf("      Timer interval: %llu counts (%u Hz)\n",
                   timer_interval, TIMER_TICK_HZ);

    /* Reset tick counter */
    timer_ticks = 0;

    /* Disable timer while configuring */
    timer_write_control(0);

    /* Set initial compare value */
    uint64_t current = timer_get_counter();
    timer_write_cval(current + timer_interval);

    /* Install timer IRQ handler in GIC */
    gic_install_handler(IRQ_TIMER_PHYS, timer_irq_handler);
    gic_set_priority(IRQ_TIMER_PHYS, 0x80);  /* Medium priority */
    gic_enable_irq(IRQ_TIMER_PHYS);

    /* Enable timer: ENABLE=1, IMASK=0 (unmask interrupt) */
    timer_write_control(0x1);

    console_printf("      Timer initialized and enabled\n");
}

/**
 * Get timer frequency
 */
uint64_t timer_get_frequency(void) {
    return timer_frequency;
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
 */
void timer_sleep_ms(uint32_t ms) {
    uint64_t target_ticks = timer_get_counter() + ((uint64_t)ms * timer_frequency) / 1000;
    while (timer_get_counter() < target_ticks) {
        __asm__ volatile("nop");
    }
}
