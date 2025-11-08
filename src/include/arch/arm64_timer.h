/**
 * ARM64 Generic Timer Driver
 * Provides system tick timer using ARM Generic Timer (EL1 Physical Timer)
 */

#ifndef ARM64_TIMER_H
#define ARM64_TIMER_H

#include <kernel/types.h>

/* Timer frequency (Hz) - typically 62.5 MHz on QEMU, but read from CNTFRQ_EL0 */
#define TIMER_DEFAULT_FREQ  62500000

/* Timer tick frequency (Hz) - how often we want timer interrupts */
#define TIMER_TICK_HZ       100     /* 100 Hz = 10 ms per tick */

/**
 * Initialize ARM Generic Timer
 * - Reads timer frequency
 * - Configures EL1 Physical Timer
 * - Enables timer interrupts via GIC
 */
void timer_init(void);

/**
 * Get current timer counter value
 * @return 64-bit counter value
 */
uint64_t timer_get_counter(void);

/**
 * Get timer frequency in Hz
 * @return Frequency in Hz
 */
uint64_t timer_get_frequency(void);

/**
 * Get number of timer ticks since boot
 * @return Tick count
 */
uint64_t timer_get_ticks(void);

/**
 * Get uptime in milliseconds
 * @return Uptime in ms
 */
uint64_t timer_get_uptime_ms(void);

/**
 * Sleep for specified milliseconds (busy wait)
 * @param ms - Milliseconds to sleep
 */
void timer_sleep_ms(uint32_t ms);

/**
 * Timer interrupt handler (called from GIC)
 */
void timer_irq_handler(void);

#endif /* ARM64_TIMER_H */
