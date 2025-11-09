#include <kernel/console.h>
#include <kernel/types.h>
#include <kernel/mm.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <arch/interrupts.h>
#include <arch/arm64_mmu.h>
#include <arch/arm64_timer.h>

extern char uart_getchar(void);
extern bool uart_has_data(void);

/* Linker-provided symbols */
extern char _kernel_end[];

void kernel_main(void) {
    /* Initialize console (UART) */
    console_init();

    /* Print welcome message */
    console_printf("========================================\n");
    console_printf("  Zixiao Server OS - ARM64 Edition\n");
    console_printf("========================================\n\n");

    console_printf("Kernel loaded successfully!\n");
    console_printf("Architecture: ARM64 (AArch64)\n");
    console_printf("Running at EL1 (Kernel mode)\n\n");

    console_printf("Initializing subsystems...\n");

    /* Initialize Physical Memory Manager */
    console_printf("  [*] Initializing physical memory manager...\n");
    uint64_t mem_start = (uint64_t)_kernel_end;
    uint64_t mem_end = 0x60000000;  /* QEMU virt: 512MB at 0x40000000-0x60000000 */
    pmm_init(mem_start, mem_end);
    console_printf("      PMM: %llu MB available (%llu pages)\n",
                   (pmm_get_free_pages() * PAGE_SIZE) / (1024 * 1024),
                   pmm_get_free_pages());

    /* Initialize MMU and virtual memory */
    console_printf("  [*] Initializing MMU and page tables...\n");
    arm64_mmu_init();

    /* Initialize interrupts */
    console_printf("  [*] Initializing exception handlers...\n");
    interrupts_init();

    /* Initialize timer */
    console_printf("  [*] Initializing Generic Timer...\n");
    timer_init();

    /* Enable interrupts */
    console_printf("  [*] Enabling interrupts...\n");
    interrupts_enable();

    console_printf("\nAll systems initialized!\n\n");

    /* Test PMM allocation */
    console_printf("Testing PMM allocation...\n");
    void* page1 = pmm_alloc_page();
    void* page2 = pmm_alloc_page();
    void* page3 = pmm_alloc_page();
    console_printf("  Allocated pages: 0x%llx, 0x%llx, 0x%llx\n",
                   (uint64_t)page1, (uint64_t)page2, (uint64_t)page3);
    pmm_free_page(page2);
    console_printf("  Freed page: 0x%llx\n", (uint64_t)page2);
    void* page4 = pmm_alloc_page();
    console_printf("  Re-allocated page: 0x%llx (should reuse freed page)\n", (uint64_t)page4);
    console_printf("  Free pages remaining: %llu\n\n", pmm_get_free_pages());

    /* Initialize kernel heap */
    console_printf("Initializing kernel heap...\n");
    #define HEAP_SIZE_PAGES 1024  /* 4MB heap */
    void* heap_pages[HEAP_SIZE_PAGES];
    for (int i = 0; i < HEAP_SIZE_PAGES; i++) {
        heap_pages[i] = pmm_alloc_page();
        if (heap_pages[i] == NULL) {
            console_printf("  ERROR: Failed to allocate heap page %d\n", i);
            while(1);
        }
    }
    /* Use first allocated page as heap start */
    void* heap_start = heap_pages[0];
    kmalloc_init(heap_start, HEAP_SIZE_PAGES * PAGE_SIZE);

    uint64_t total, used, free_heap;
    kmalloc_stats(&total, &used, &free_heap);
    console_printf("  Heap: %llu KB total\n\n", total / 1024);

    /* Test kmalloc/kfree */
    console_printf("Testing kmalloc/kfree...\n");
    char* str1 = (char*)kmalloc(64);
    char* str2 = (char*)kmalloc(128);
    char* str3 = (char*)kmalloc(256);
    console_printf("  Allocated: 0x%llx (64B), 0x%llx (128B), 0x%llx (256B)\n",
                   (uint64_t)str1, (uint64_t)str2, (uint64_t)str3);

    /* Write some data */
    if (str1) {
        for (int i = 0; i < 10; i++) str1[i] = 'A' + i;
        str1[10] = '\0';
        console_printf("  str1 = %s\n", str1);
    }

    kfree(str2);
    console_printf("  Freed str2\n");

    char* str4 = (char*)kmalloc(100);
    console_printf("  Re-allocated: 0x%llx (100B)\n", (uint64_t)str4);

    kmalloc_stats(&total, &used, &free_heap);
    console_printf("  Heap usage: %llu KB used, %llu KB free\n\n",
                   used / 1024, free_heap / 1024);

    /* Initialize Yuheng scheduler */
    console_printf("Initializing Yuheng (玉衡) scheduler...\n");

    /* Disable interrupts during scheduler init to avoid timer interference */
    __asm__ volatile("msr daifset, #2");  /* Disable IRQ */

    scheduler_init();

    /* Re-enable interrupts */
    __asm__ volatile("msr daifclr, #2");  /* Enable IRQ */

    /* Create test tasks */
    extern void test_task_a(void);
    extern void test_task_b(void);
    extern void test_task_c(void);

    console_printf("Creating test tasks...\n");
    task_struct_t* task_a = task_create("test_a", test_task_a, 5, 8192);
    task_struct_t* task_b = task_create("test_b", test_task_b, 5, 8192);
    task_struct_t* task_c = task_create("test_c", test_task_c, 3, 8192);

    /* Add tasks to ready queue */
    task_ready(task_a);
    task_ready(task_b);
    task_ready(task_c);

    console_printf("\nScheduler ready! Starting task execution...\n");
    console_printf("(Press 'p' to test kernel panic, Ctrl+A then X to exit QEMU)\n\n");

    /* Become the idle task - enter infinite loop with WFI */
    console_printf("Entering idle loop (kernel_main becomes idle task)...\n\n");

    while (1) {
        /* Check for UART input to trigger panic test */
        if (uart_has_data()) {
            char c = uart_getchar();
            if (c == 'p' || c == 'P') {
                /* Trigger a test kernel panic */
                console_printf("\n\n*** User requested kernel panic test ***\n");

                /* Prepare register dump (current state) */
                panic_regs_t regs;
                __builtin_memset(&regs, 0, sizeof(regs));

                /* Capture current registers */
                uint64_t sp, lr, pc;
                __asm__ volatile("mov %0, sp" : "=r"(sp));
                __asm__ volatile("mov %0, x30" : "=r"(lr));
                __asm__ volatile("adr %0, ." : "=r"(pc));

                regs.sp = sp;
                regs.lr = lr;
                regs.pc = pc;

                /* Read processor state */
                __asm__ volatile("mrs %0, NZCV" : "=r"(regs.flags));

                /* Trigger panic with a demonstration message */
                kernel_panic("User-triggered test panic", &regs);
            }
        }

        /* Wait for interrupt - scheduler will be triggered by timer */
        __asm__ volatile("wfi");
    }
}
