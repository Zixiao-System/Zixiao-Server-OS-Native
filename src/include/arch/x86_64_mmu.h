/**
 * x86_64 MMU and Page Table Management
 * Implements 4-level page tables (PML4→PDPT→PD→PT) for 48-bit virtual addresses
 */

#ifndef X86_64_MMU_H
#define X86_64_MMU_H

#include <kernel/types.h>

/* Page table pointer type */
typedef uint64_t* page_table_t;

/* Page size (4KB) */
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12

/* Page Table Entry Flags (x86_64 specific) */
#define PTE_PRESENT     (1ULL << 0)    /* Present in memory */
#define PTE_WRITE       (1ULL << 1)    /* Read/Write (0=read-only, 1=read/write) */
#define PTE_USER        (1ULL << 2)    /* User/Supervisor (0=supervisor, 1=user) */
#define PTE_WRITETHROUGH (1ULL << 3)   /* Write-through caching */
#define PTE_NOCACHE     (1ULL << 4)    /* Cache disable */
#define PTE_ACCESSED    (1ULL << 5)    /* Accessed (set by CPU) */
#define PTE_DIRTY       (1ULL << 6)    /* Dirty (set by CPU on write) */
#define PTE_LARGE       (1ULL << 7)    /* Large page (2MB at PD level, 1GB at PDPT level) */
#define PTE_GLOBAL      (1ULL << 8)    /* Global page (not flushed from TLB on CR3 reload) */
#define PTE_NX          (1ULL << 63)   /* No-execute (execution disable) */

/* Custom flag for device memory (not a real PTE flag, cleared before use) */
#define PTE_DEVICE      (0x100ULL)

/* Physical address mask (bits 12-51, assuming 52-bit physical addressing) */
#define PTE_ADDR_MASK   0x000FFFFFFFFFF000ULL

/* Page table indexing macros (each level has 9 bits, 512 entries) */
#define PML4_INDEX(va)  (((va) >> 39) & 0x1FF)  /* Bits [47:39] */
#define PDPT_INDEX(va)  (((va) >> 30) & 0x1FF)  /* Bits [38:30] */
#define PD_INDEX(va)    (((va) >> 21) & 0x1FF)  /* Bits [29:21] */
#define PT_INDEX(va)    (((va) >> 12) & 0x1FF)  /* Bits [20:12] */

/* Function declarations */

/**
 * Initialize x86_64 MMU
 * - Creates kernel page tables
 * - Identity maps kernel region
 * - Maps VGA buffer as device memory
 * - Enables paging via CR0.PG
 */
void x86_64_mmu_init(void);

/**
 * Create a new empty page table
 * @return Pointer to PML4 table, or NULL on failure
 */
page_table_t x86_64_create_page_table(void);

/**
 * Map a virtual page to a physical page
 * @param pml4 - Page table root (PML4)
 * @param virt_addr - Virtual address (will be page-aligned)
 * @param phys_addr - Physical address (will be page-aligned)
 * @param flags - PTE flags (PRESENT, WRITE, USER, NOCACHE, etc.)
 * @return 0 on success, -1 on failure
 */
int x86_64_map_page(page_table_t pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

/**
 * Unmap a virtual page
 * @param pml4 - Page table root (PML4)
 * @param virt_addr - Virtual address to unmap
 */
void x86_64_unmap_page(page_table_t pml4, uint64_t virt_addr);

/**
 * Switch to a different page table (load CR3)
 * @param pml4 - New page table root
 */
void x86_64_switch_page_table(page_table_t pml4);

/**
 * Get current page table base from CR3
 * @return Physical address of current PML4
 */
uint64_t x86_64_get_current_page_table(void);

#endif /* X86_64_MMU_H */
