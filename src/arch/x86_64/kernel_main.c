#include <kernel/console.h>
#include <kernel/types.h>
#include <kernel/mm.h>
#include <kernel/string.h>
#include <kernel/sched.h>
#include <arch/interrupts.h>
#include <arch/x86_64_mmu.h>
#include <arch/x86_64_timer.h>

/* External initialization functions */
extern void gdt_init(void);
extern void keyboard_init(void);

/* Linker-provided symbol: end of kernel */
extern char _kernel_end[];

void kernel_main(uint32_t magic, uint32_t mbi_addr) {
    /* Initialize console */
    console_init();

    /* Print welcome message */
    console_printf("========================================\n");
    console_printf("  Zixiao Server OS - x86_64 Edition\n");
    console_printf("========================================\n\n");

    console_printf("Kernel loaded successfully!\n");
    console_printf("Multiboot2 magic: 0x%x\n", magic);
    console_printf("MBI address: 0x%x\n\n", mbi_addr);

    console_printf("Architecture: x86_64 (Long Mode)\n");
    console_printf("Status: Running in 64-bit mode\n\n");

    console_printf("Initializing subsystems...\n");

    /* Initialize physical memory manager */
    console_printf("  [*] Initializing physical memory manager...\n");
    uint64_t mem_start = (uint64_t)_kernel_end;
    uint64_t mem_end = 0x20000000;  /* 512 MB total memory */
    pmm_init(mem_start, mem_end);
    console_printf("      PMM: %llu MB available (%llu pages)\n",
                   (mem_end - mem_start) / (1024 * 1024),
                   pmm_get_free_pages());

    /* Initialize MMU and page tables */
    console_printf("  [*] Initializing MMU and page tables...\n");
    x86_64_mmu_init();

    /* Initialize GDT */
    console_printf("  [*] Initializing GDT...\n");
    gdt_init();

    /* Initialize interrupts */
    console_printf("  [*] Initializing IDT...\n");
    interrupts_init();

    /* Initialize keyboard driver */
    console_printf("  [*] Initializing keyboard driver...\n");
    keyboard_init();

    /* Initialize timer */
    console_printf("  [*] Initializing PIT timer...\n");
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
    #define HEAP_SIZE_PAGES 1024  /* 4 MB heap */
    void* heap_pages[HEAP_SIZE_PAGES];
    for (int i = 0; i < HEAP_SIZE_PAGES; i++) {
        heap_pages[i] = pmm_alloc_page();
        if (heap_pages[i] == NULL) {
            console_printf("ERROR: Failed to allocate heap page %d\n", i);
            while(1);
        }
    }
    kmalloc_init(heap_pages[0], HEAP_SIZE_PAGES * PAGE_SIZE);
    console_printf("  Heap: %llu KB total\n\n", (HEAP_SIZE_PAGES * PAGE_SIZE) / 1024);

    /* Test kmalloc/kfree */
    console_printf("Testing kmalloc/kfree...\n");
    char* str1 = (char*)kmalloc(64);
    char* str2 = (char*)kmalloc(128);
    char* str3 = (char*)kmalloc(256);
    console_printf("  Allocated: 0x%llx (64B), 0x%llx (128B), 0x%llx (256B)\n",
                   (uint64_t)str1, (uint64_t)str2, (uint64_t)str3);

    /* Write to str1 to test */
    memcpy(str1, "ABCDEFGHIJ", 11);
    console_printf("  str1 = %s\n", str1);

    kfree(str2);
    console_printf("  Freed str2\n");

    char* str4 = (char*)kmalloc(100);
    console_printf("  Re-allocated: 0x%llx (100B)\n", (uint64_t)str4);

    uint64_t total, used, free_heap;
    kmalloc_stats(&total, &used, &free_heap);
    console_printf("  Heap usage: %llu KB used, %llu KB free\n\n",
                   used / 1024, free_heap / 1024);

    /* Initialize Yuheng scheduler */
    console_printf("Initializing Yuheng (玉衡) scheduler...\n");
    scheduler_init();

    /* Create test tasks */
    extern void test_task_a(void);
    extern void test_task_b(void);
    extern void test_task_c(void);

    console_printf("Creating test tasks...\n");
    task_create("test_a", test_task_a, 5, 8192);
    task_create("test_b", test_task_b, 5, 8192);
    task_create("test_c", test_task_c, 3, 8192);

    console_printf("\nScheduler ready! Starting task execution...\n");
    console_printf("(Press Ctrl+C in terminal to exit QEMU)\n\n");

    /* Start scheduling - this will never return */
    schedule();

    /* Should never reach here */
    console_printf("ERROR: Scheduler returned!\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}
