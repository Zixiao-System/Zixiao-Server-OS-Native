#ifndef ZIXIAO_MM_H
#define ZIXIAO_MM_H

#include <kernel/types.h>

/* Physical Memory Manager (PMM) */

#define PAGE_SIZE 4096  /* 4KB pages */

/**
 * Initialize the physical memory manager
 * @param mem_start - Start of available physical memory
 * @param mem_end - End of available physical memory
 */
void pmm_init(uint64_t mem_start, uint64_t mem_end);

/**
 * Allocate a single physical page (4KB)
 * @return Physical address of allocated page, or 0 if out of memory
 */
void* pmm_alloc_page(void);

/**
 * Free a previously allocated physical page
 * @param page - Physical address of page to free
 */
void pmm_free_page(void* page);

/**
 * Get the number of free pages available
 * @return Number of free 4KB pages
 */
uint64_t pmm_get_free_pages(void);

/**
 * Get total number of pages managed by PMM
 * @return Total number of 4KB pages
 */
uint64_t pmm_get_total_pages(void);

/* Kernel Heap Allocator (kmalloc) */

/**
 * Allocate kernel heap memory
 * @param size - Number of bytes to allocate
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
void* kmalloc(size_t size);

/**
 * Allocate aligned kernel heap memory
 * @param size - Number of bytes to allocate
 * @param align - Alignment requirement (must be power of 2)
 * @return Pointer to allocated memory, or NULL if allocation fails
 */
void* kmalloc_aligned(size_t size, uint32_t align);

/**
 * Free previously allocated kernel heap memory
 * @param ptr - Pointer to memory to free
 */
void kfree(void* ptr);

/**
 * Initialize the kernel heap allocator
 * @param heap_start - Start address of heap region
 * @param heap_size - Size of heap region in bytes
 */
void kmalloc_init(void* heap_start, size_t heap_size);

/**
 * Get heap usage statistics
 * @param total - Total heap size (can be NULL)
 * @param used - Used heap size (can be NULL)
 * @param free - Free heap size (can be NULL)
 */
void kmalloc_stats(uint64_t* total, uint64_t* used, uint64_t* free);

#endif // ZIXIAO_MM_H
