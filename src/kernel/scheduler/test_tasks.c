/**
 * Test Tasks for Scheduler Verification
 */

#include <kernel/sched.h>
#include <kernel/console.h>

void test_task_a(void) {
    for (int i = 0; i < 10; i++) {
        console_printf("[Task A] Running (iteration %d)\n", i);
        task_yield();
    }
    console_printf("[Task A] Completed\n");
    task_exit();
}

void test_task_b(void) {
    for (int i = 0; i < 10; i++) {
        console_printf("[Task B] Running (iteration %d)\n", i);
        task_yield();
    }
    console_printf("[Task B] Completed\n");
    task_exit();
}

void test_task_c(void) {
    int i = 0;
    while (1) {
        console_printf("[Task C] Running (iteration %d)\n", i++);
        task_yield();
    }
}
