#ifndef ZIXIAO_ARCH_ARM64_MMU_H
#define ZIXIAO_ARCH_ARM64_MMU_H

#include <kernel/types.h>

/* ARM64 Page Table Entry (PTE) flags */
#define PTE_VALID       (1ULL << 0)   /* Valid entry */
#define PTE_TABLE       (1ULL << 1)   /* Page table (not block) */
#define PTE_USER        (1ULL << 6)   /* User accessible (AP[1]) */
#define PTE_READONLY    (1ULL << 7)   /* Read-only (AP[2]) */
#define PTE_ACCESSED    (1ULL << 10)  /* Accessed flag */
#define PTE_NG          (1ULL << 11)  /* Not global */

/* Memory attributes index (MAIR_EL1) */
#define MT_DEVICE_nGnRnE  0  /* Device memory, non-gathering, non-reordering, no early write ack */
#define MT_NORMAL_NC      1  /* Normal memory, non-cacheable */
#define MT_NORMAL         2  /* Normal memory, cacheable */

#define PTE_ATTR_SHIFT    2
#define PTE_ATTR(n)       ((uint64_t)(n) << PTE_ATTR_SHIFT)

/* Convenience flag to specify device memory in map_page flags */
#define PTE_DEVICE        (0x100ULL)  /* Bit 8 - indicates device memory */

/* Address masks */
#define PTE_ADDR_MASK     0x0000FFFFFFFFF000ULL
#define PTE_TABLE_MASK    0x0000FFFFFFFFF000ULL

/* Page table levels */
#define PGLEVEL_4K_0      0   /* PGD - 512GB per entry */
#define PGLEVEL_4K_1      1   /* PUD - 1GB per entry */
#define PGLEVEL_4K_2      2   /* PMD - 2MB per entry */
#define PGLEVEL_4K_3      3   /* PTE - 4KB per entry */

/* Virtual address bit fields for 48-bit VA, 4KB pages */
#define VA_BITS           48
#define PGDIR_SHIFT       39  /* Bits for level 0 (PGD) */
#define PUD_SHIFT         30  /* Bits for level 1 (PUD) */
#define PMD_SHIFT         21  /* Bits for level 2 (PMD) */
#define PAGE_SHIFT        12  /* Bits for level 3 (PTE) */

#define PTRS_PER_TABLE    512 /* 512 entries per table */
#define TABLE_SIZE        (PTRS_PER_TABLE * sizeof(uint64_t))

/* Extract index for each level */
#define PGD_INDEX(va)     (((va) >> PGDIR_SHIFT) & 0x1FF)
#define PUD_INDEX(va)     (((va) >> PUD_SHIFT) & 0x1FF)
#define PMD_INDEX(va)     (((va) >> PMD_SHIFT) & 0x1FF)
#define PTE_INDEX(va)     (((va) >> PAGE_SHIFT) & 0x1FF)

/* Page table types */
typedef uint64_t* page_table_t;

/**
 * Initialize ARM64 MMU and page tables
 */
void arm64_mmu_init(void);

/**
 * Create a new empty page table (for processes)
 * @return Pointer to new page table, or NULL on failure
 */
page_table_t arm64_create_page_table(void);

/**
 * Map a virtual page to a physical page
 * @param pgd - Page global directory (level 0)
 * @param virt_addr - Virtual address to map
 * @param phys_addr - Physical address to map to
 * @param flags - Page flags (PTE_USER, PTE_READONLY, etc.)
 * @return 0 on success, -1 on failure
 */
int arm64_map_page(page_table_t pgd, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);

/**
 * Unmap a virtual page
 * @param pgd - Page global directory
 * @param virt_addr - Virtual address to unmap
 */
void arm64_unmap_page(page_table_t pgd, uint64_t virt_addr);

/**
 * Switch to a different page table
 * @param pgd - Page global directory physical address
 */
void arm64_switch_page_table(page_table_t pgd);

/**
 * Get current page table base address
 * @return Physical address of current TTBR0_EL1
 */
uint64_t arm64_get_current_page_table(void);

#endif // ZIXIAO_ARCH_ARM64_MMU_H
