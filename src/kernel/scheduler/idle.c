/**
 * Idle Task - Runs when no other tasks ready
 */

#include <kernel/sched.h>

void idle_task_entry(void) {
    while (1) {
        /* ARM64: Wait for interrupt */
        __asm__ volatile("wfi");
    }
}
