/**
 * x86_64 PIT (Programmable Interval Timer - 8254) Driver
 * Provides system tick timer using Intel 8254 PIT
 */

#ifndef X86_64_TIMER_H
#define X86_64_TIMER_H

#include <kernel/types.h>

/* PIT Hardware Constants (Intel 8254 chip) */
#define PIT_CHANNEL0    0x40    /* Channel 0 data port (system timer) */
#define PIT_CHANNEL1    0x41    /* Channel 1 data port (unused) */
#define PIT_CHANNEL2    0x42    /* Channel 2 data port (PC speaker) */
#define PIT_COMMAND     0x43    /* Mode/Command register */

/* PIT Base Frequency (approximately 1.193182 MHz) */
#define PIT_BASE_FREQ   1193182

/* Timer tick frequency (Hz) - how often we want timer interrupts */
#define TIMER_TICK_HZ   100     /* 100 Hz = 10 ms per tick */

/* PIT Command Register bits */
#define PIT_CMD_BINARY      0x00    /* Use binary counter values */
#define PIT_CMD_BCD         0x01    /* Use BCD counter values */
#define PIT_CMD_MODE0       0x00    /* Mode 0: Interrupt on terminal count */
#define PIT_CMD_MODE1       0x02    /* Mode 1: Hardware retriggerable one-shot */
#define PIT_CMD_MODE2       0x04    /* Mode 2: Rate generator */
#define PIT_CMD_MODE3       0x06    /* Mode 3: Square wave generator */
#define PIT_CMD_MODE4       0x08    /* Mode 4: Software triggered strobe */
#define PIT_CMD_MODE5       0x0A    /* Mode 5: Hardware triggered strobe */
#define PIT_CMD_LATCH       0x00    /* Latch count value command */
#define PIT_CMD_LOBYTE      0x10    /* Read/Write low byte only */
#define PIT_CMD_HIBYTE      0x20    /* Read/Write high byte only */
#define PIT_CMD_LOHI        0x30    /* Read/Write low byte then high byte */
#define PIT_CMD_CHANNEL0    0x00    /* Select channel 0 */
#define PIT_CMD_CHANNEL1    0x40    /* Select channel 1 */
#define PIT_CMD_CHANNEL2    0x80    /* Select channel 2 */

/**
 * Initialize PIT timer
 * - Configures channel 0 in mode 2 (rate generator)
 * - Sets up for TIMER_TICK_HZ interrupts per second
 * - Enables IRQ 0 via PIC
 */
void timer_init(void);

/**
 * Get timer frequency in Hz
 * @return Frequency in Hz (always PIT_BASE_FREQ for x86_64)
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
 * NOTE: This is a busy-wait implementation. More efficient sleep
 *       will be available after scheduler is implemented.
 */
void timer_sleep_ms(uint32_t ms);

/**
 * Timer interrupt handler (called from IRQ 0)
 * This is called by the IDT IRQ dispatcher
 */
void timer_irq_handler(void);

#endif /* X86_64_TIMER_H */
