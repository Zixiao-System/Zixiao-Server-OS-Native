/**
 * Physical Memory Manager (PMM)
 * Manages physical memory using a bitmap allocator
 * Each bit represents one 4KB page (0 = free, 1 = allocated)
 */

#include <kernel/mm.h>
#include <kernel/string.h>

/* Bitmap to track page allocation status */
static uint64_t* pmm_bitmap = NULL;
static uint64_t pmm_total_pages = 0;
static uint64_t pmm_free_pages = 0;
static uint64_t pmm_mem_start = 0;
static uint64_t pmm_mem_end = 0;

/* Bitmap manipulation macros */
#define BITMAP_INDEX(page) ((page) / 64)
#define BITMAP_OFFSET(page) ((page) % 64)
#define BITMAP_SET(page) (pmm_bitmap[BITMAP_INDEX(page)] |= (1ULL << BITMAP_OFFSET(page)))
#define BITMAP_CLEAR(page) (pmm_bitmap[BITMAP_INDEX(page)] &= ~(1ULL << BITMAP_OFFSET(page)))
#define BITMAP_TEST(page) (pmm_bitmap[BITMAP_INDEX(page)] & (1ULL << BITMAP_OFFSET(page)))

/**
 * Initialize the physical memory manager
 */
void pmm_init(uint64_t mem_start, uint64_t mem_end)
{
    /* Align memory boundaries to page size */
    mem_start = (mem_start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    mem_end = mem_end & ~(PAGE_SIZE - 1);

    pmm_mem_start = mem_start;
    pmm_mem_end = mem_end;

    /* Calculate total number of pages */
    uint64_t mem_size = mem_end - mem_start;
    pmm_total_pages = mem_size / PAGE_SIZE;

    /* Calculate bitmap size (in bytes) */
    uint64_t bitmap_size = (pmm_total_pages + 7) / 8;  /* Round up to nearest byte */
    bitmap_size = (bitmap_size + 7) & ~7;  /* Align to 8 bytes */

    /* Place bitmap at start of available memory */
    pmm_bitmap = (uint64_t*)mem_start;

    /* Zero out the bitmap (mark all pages as free initially) */
    memset(pmm_bitmap, 0, bitmap_size);

    /* Mark pages used by bitmap itself as allocated */
    uint64_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        BITMAP_SET(i);
    }

    /* Set free page count */
    pmm_free_pages = pmm_total_pages - bitmap_pages;
}

/**
 * Allocate a single physical page
 */
void* pmm_alloc_page(void)
{
    if (pmm_free_pages == 0) {
        return NULL;  /* Out of memory */
    }

    /* Search for a free page in the bitmap */
    for (uint64_t i = 0; i < pmm_total_pages; i++) {
        if (!BITMAP_TEST(i)) {
            /* Found a free page */
            BITMAP_SET(i);
            pmm_free_pages--;

            /* Calculate physical address of this page */
            uint64_t phys_addr = pmm_mem_start + (i * PAGE_SIZE);
            return (void*)phys_addr;
        }
    }

    /* Should never reach here if pmm_free_pages was accurate */
    return NULL;
}

/**
 * Free a previously allocated physical page
 */
void pmm_free_page(void* page)
{
    if (page == NULL) {
        return;
    }

    uint64_t phys_addr = (uint64_t)page;

    /* Validate that address is within managed range */
    if (phys_addr < pmm_mem_start || phys_addr >= pmm_mem_end) {
        return;  /* Invalid address */
    }

    /* Calculate page index */
    uint64_t page_index = (phys_addr - pmm_mem_start) / PAGE_SIZE;

    /* Check if page is actually allocated */
    if (!BITMAP_TEST(page_index)) {
        return;  /* Double free or invalid */
    }

    /* Mark page as free */
    BITMAP_CLEAR(page_index);
    pmm_free_pages++;
}

/**
 * Get the number of free pages
 */
uint64_t pmm_get_free_pages(void)
{
    return pmm_free_pages;
}

/**
 * Get total number of pages
 */
uint64_t pmm_get_total_pages(void)
{
    return pmm_total_pages;
}
