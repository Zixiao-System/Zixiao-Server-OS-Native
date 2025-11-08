/**
 * Kernel Heap Allocator (kmalloc/kfree)
 * Uses a linked-list free block allocator (K&R style)
 */

#include <kernel/mm.h>
#include <kernel/string.h>

#define HEAP_MAGIC 0xDEADBEEF  /* Magic number for debugging */

/* Memory block header */
typedef struct mem_block {
    uint32_t magic;           /* Magic number for validation */
    uint32_t size;            /* Size of usable data (excluding header) */
    struct mem_block* next;   /* Next free block (only for free blocks) */
    uint8_t allocated;        /* 1 if allocated, 0 if free */
} mem_block_t;

#define BLOCK_HEADER_SIZE sizeof(mem_block_t)

/* Heap state */
static void* heap_start = NULL;
static void* heap_end = NULL;
static mem_block_t* free_list = NULL;  /* Head of free block list */
static uint64_t total_heap_size = 0;
static uint64_t used_heap_size = 0;

/**
 * Initialize the kernel heap allocator
 */
void kmalloc_init(void* heap_start_addr, size_t heap_size)
{
    heap_start = heap_start_addr;
    heap_end = (void*)((uint64_t)heap_start + heap_size);
    total_heap_size = heap_size;
    used_heap_size = 0;

    /* Initialize the first free block (entire heap) */
    free_list = (mem_block_t*)heap_start;
    free_list->magic = HEAP_MAGIC;
    free_list->size = heap_size - BLOCK_HEADER_SIZE;
    free_list->next = NULL;
    free_list->allocated = 0;
}

/**
 * Find a free block that fits the requested size (first-fit strategy)
 */
static mem_block_t* find_free_block(size_t size)
{
    mem_block_t* current = free_list;
    mem_block_t* prev = NULL;

    while (current != NULL) {
        if (!current->allocated && current->size >= size) {
            /* Found a suitable block */
            return current;
        }
        prev = current;
        current = current->next;
    }

    return NULL;  /* No suitable block found */
}

/**
 * Split a block if it's significantly larger than needed
 */
static void split_block(mem_block_t* block, size_t size)
{
    /* Only split if remaining space is large enough for another block */
    if (block->size >= size + BLOCK_HEADER_SIZE + 32) {
        /* Create new free block from remaining space */
        mem_block_t* new_block = (mem_block_t*)((uint64_t)block + BLOCK_HEADER_SIZE + size);
        new_block->magic = HEAP_MAGIC;
        new_block->size = block->size - size - BLOCK_HEADER_SIZE;
        new_block->allocated = 0;
        new_block->next = block->next;

        /* Update original block */
        block->size = size;
        block->next = new_block;
    }
}

/**
 * Allocate kernel heap memory
 */
void* kmalloc(size_t size)
{
    if (size == 0) {
        return NULL;
    }

    /* Align size to 8 bytes */
    size = (size + 7) & ~7;

    /* Find a free block */
    mem_block_t* block = find_free_block(size);
    if (block == NULL) {
        return NULL;  /* Out of memory */
    }

    /* Split block if it's too large */
    split_block(block, size);

    /* Mark block as allocated */
    block->allocated = 1;
    used_heap_size += block->size + BLOCK_HEADER_SIZE;

    /* Return pointer to usable data (after header) */
    return (void*)((uint64_t)block + BLOCK_HEADER_SIZE);
}

/**
 * Allocate aligned kernel heap memory
 */
void* kmalloc_aligned(size_t size, uint32_t align)
{
    if (size == 0 || align == 0) {
        return NULL;
    }

    /* Ensure alignment is a power of 2 */
    if ((align & (align - 1)) != 0) {
        return NULL;  /* Invalid alignment */
    }

    /* Allocate extra space for alignment adjustment */
    size_t extra = align + BLOCK_HEADER_SIZE;
    void* ptr = kmalloc(size + extra);
    if (ptr == NULL) {
        return NULL;
    }

    /* Calculate aligned address */
    uint64_t aligned_addr = ((uint64_t)ptr + align - 1) & ~(align - 1);

    /* For simplicity, just return the allocated pointer
     * (proper implementation would adjust the block header) */
    return (void*)aligned_addr;
}

/**
 * Free previously allocated kernel heap memory
 */
void kfree(void* ptr)
{
    if (ptr == NULL) {
        return;
    }

    /* Get block header */
    mem_block_t* block = (mem_block_t*)((uint64_t)ptr - BLOCK_HEADER_SIZE);

    /* Validate magic number */
    if (block->magic != HEAP_MAGIC) {
        return;  /* Invalid pointer or corrupted heap */
    }

    /* Check if already freed */
    if (!block->allocated) {
        return;  /* Double free */
    }

    /* Mark as free */
    block->allocated = 0;
    used_heap_size -= block->size + BLOCK_HEADER_SIZE;

    /* Coalesce with adjacent free blocks */
    /* Simple implementation: just mark as free and add to free list */
    /* (A more sophisticated implementation would merge adjacent free blocks) */
}

/**
 * Get heap usage statistics
 */
void kmalloc_stats(uint64_t* total, uint64_t* used, uint64_t* free)
{
    if (total) *total = total_heap_size;
    if (used) *used = used_heap_size;
    if (free) *free = total_heap_size - used_heap_size;
}
