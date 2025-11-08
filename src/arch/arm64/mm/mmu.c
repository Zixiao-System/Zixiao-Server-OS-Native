/**
 * ARM64 MMU and Page Table Management
 * Implements 4-level page tables for 48-bit virtual addresses with 4KB pages
 */

#include <arch/arm64_mmu.h>
#include <kernel/mm.h>
#include <kernel/string.h>
#include <kernel/console.h>

/* Kernel page table (shared across all processes) */
static page_table_t kernel_pgd = NULL;

/**
 * Helper: Allocate a zeroed page table
 */
static uint64_t* alloc_page_table(void)
{
    void* page = pmm_alloc_page();
    if (page == NULL) {
        return NULL;
    }
    memset(page, 0, PAGE_SIZE);
    return (uint64_t*)page;
}

/**
 * Helper: Get or create page table at specified level
 * @param table - Parent table
 * @param index - Index in parent table
 * @param is_leaf - Whether this is the final level (leaf)
 * @return Pointer to next level table, or NULL on failure
 */
static uint64_t* get_or_create_table(uint64_t* table, uint64_t index, int is_leaf)
{
    uint64_t entry = table[index];

    /* Check if entry already exists */
    if (entry & PTE_VALID) {
        if (!is_leaf && (entry & PTE_TABLE)) {
            /* Return existing table */
            return (uint64_t*)(entry & PTE_TABLE_MASK);
        } else if (is_leaf) {
            /* Entry already mapped */
            return table;
        } else {
            /* Conflict: entry is a block when we need a table */
            return NULL;
        }
    }

    /* Create new table if not leaf level */
    if (!is_leaf) {
        uint64_t* new_table = alloc_page_table();
        if (new_table == NULL) {
            return NULL;
        }

        /* Install table descriptor */
        table[index] = ((uint64_t)new_table & PTE_TABLE_MASK) | PTE_TABLE | PTE_VALID;
        return new_table;
    }

    /* Leaf level - caller will install actual mapping */
    return table;
}

/**
 * Map a virtual page to a physical page
 */
int arm64_map_page(page_table_t pgd, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags)
{
    /* Ensure addresses are page-aligned */
    virt_addr &= ~(PAGE_SIZE - 1);
    phys_addr &= ~(PAGE_SIZE - 1);

    /* Debug: Only print for special addresses to avoid spam */
    if (virt_addr == 0x09000000) {
        console_printf("        [DEBUG] Mapping 0x%llx -> 0x%llx\n", virt_addr, phys_addr);
    }

    /* Walk through page table levels */
    uint64_t* pud = get_or_create_table(pgd, PGD_INDEX(virt_addr), 0);
    if (pud == NULL) return -1;

    uint64_t* pmd = get_or_create_table(pud, PUD_INDEX(virt_addr), 0);
    if (pmd == NULL) return -1;

    uint64_t* pte_table = get_or_create_table(pmd, PMD_INDEX(virt_addr), 0);
    if (pte_table == NULL) return -1;

    /* Install leaf PTE */
    uint64_t pte_index = PTE_INDEX(virt_addr);

    /* Check if this is device memory or normal memory */
    uint64_t mem_type = (flags & PTE_DEVICE) ? MT_DEVICE_nGnRnE : MT_NORMAL;

    /* Build PTE (clear PTE_DEVICE bit from flags as it's not a real PTE flag) */
    /* At level 3, descriptor must have bits[1:0]=11 for valid page descriptor */
    uint64_t pte = phys_addr | (flags & ~PTE_DEVICE) | PTE_VALID | PTE_TABLE | PTE_ACCESSED;
    pte |= PTE_ATTR(mem_type);

    pte_table[pte_index] = pte;

    return 0;
}

/**
 * Unmap a virtual page
 */
void arm64_unmap_page(page_table_t pgd, uint64_t virt_addr)
{
    virt_addr &= ~(PAGE_SIZE - 1);

    /* Walk page tables */
    uint64_t entry = pgd[PGD_INDEX(virt_addr)];
    if (!(entry & PTE_VALID)) return;
    uint64_t* pud = (uint64_t*)(entry & PTE_TABLE_MASK);

    entry = pud[PUD_INDEX(virt_addr)];
    if (!(entry & PTE_VALID)) return;
    uint64_t* pmd = (uint64_t*)(entry & PTE_TABLE_MASK);

    entry = pmd[PMD_INDEX(virt_addr)];
    if (!(entry & PTE_VALID)) return;
    uint64_t* pte_table = (uint64_t*)(entry & PTE_TABLE_MASK);

    /* Clear PTE */
    pte_table[PTE_INDEX(virt_addr)] = 0;

    /* Invalidate TLB for this address */
    __asm__ volatile("tlbi vaae1, %0" :: "r"(virt_addr >> PAGE_SHIFT));
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");
}

/**
 * Create a new empty page table
 */
page_table_t arm64_create_page_table(void)
{
    return alloc_page_table();
}

/**
 * Switch to a different page table (TTBR0_EL1)
 */
void arm64_switch_page_table(page_table_t pgd)
{
    uint64_t ttbr0 = (uint64_t)pgd;

    /* Set TTBR0_EL1 (user space page table) */
    __asm__ volatile("msr ttbr0_el1, %0" :: "r"(ttbr0));

    /* Invalidate all TLB entries */
    __asm__ volatile("tlbi vmalle1");
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");
}

/**
 * Get current page table base
 */
uint64_t arm64_get_current_page_table(void)
{
    uint64_t ttbr0;
    __asm__ volatile("mrs %0, ttbr0_el1" : "=r"(ttbr0));
    return ttbr0;
}

/**
 * Initialize ARM64 MMU
 */
void arm64_mmu_init(void)
{
    /* Create kernel page table */
    kernel_pgd = arm64_create_page_table();
    if (kernel_pgd == NULL) {
        console_printf("ERROR: Failed to create kernel page table\n");
        while(1);
    }

    /* Identity map kernel region (only what we need, not full 512MB)
     * Map from 0x40000000 to kernel_end + 8MB for safety
     */
    console_printf("      Identity mapping kernel region (8 MB)...\n");

    /* Map only 8MB (enough for kernel + heap + stack) */
    uint64_t map_start = 0x40000000;
    uint64_t map_end = 0x40800000;   /* Only 8MB! Much faster */
    uint64_t pages_mapped = 0;

    for (uint64_t addr = map_start; addr < map_end; addr += PAGE_SIZE) {
        if (arm64_map_page(kernel_pgd, addr, addr, 0) != 0) {
            console_printf("ERROR: Failed to map 0x%llx\n", addr);
            while(1);
        }
        pages_mapped++;

        /* Print progress every 512 pages (2MB) */
        if (pages_mapped % 512 == 0) {
            console_printf("      Mapped %llu MB...\n", pages_mapped / 256);
        }
    }
    console_printf("      Mapped %llu pages (%llu MB)\n",
                   pages_mapped, pages_mapped / 256);

    /* Map UART MMIO region (0x09000000) - CRITICAL for console after MMU enable! */
    console_printf("      Mapping UART MMIO (0x09000000)...\n");
    if (arm64_map_page(kernel_pgd, 0x09000000, 0x09000000, PTE_DEVICE) != 0) {
        console_printf("ERROR: Failed to map UART\n");
        while(1);
    }
    console_printf("      UART mapped\n");

    /* Map GIC MMIO regions (0x08000000 - 0x08020000) for interrupt controller */
    console_printf("      Mapping GIC MMIO (0x08000000-0x08020000)...\n");
    for (uint64_t addr = 0x08000000; addr < 0x08020000; addr += PAGE_SIZE) {
        if (arm64_map_page(kernel_pgd, addr, addr, PTE_DEVICE) != 0) {
            console_printf("ERROR: Failed to map GIC at 0x%llx\n", addr);
            while(1);
        }
    }
    console_printf("      GIC mapped\n");

    /* Configure MAIR_EL1 (Memory Attribute Indirection Register) */
    console_printf("      Configuring MAIR_EL1...\n");
    uint64_t mair = 0;
    mair |= (0x00ULL << (MT_DEVICE_nGnRnE * 8)); /* Device memory */
    mair |= (0x44ULL << (MT_NORMAL_NC * 8));      /* Normal non-cacheable */
    mair |= (0xFFULL << (MT_NORMAL * 8));          /* Normal cacheable */
    __asm__ volatile("msr mair_el1, %0" :: "r"(mair));
    console_printf("      MAIR_EL1 configured\n");

    /* Configure TCR_EL1 (Translation Control Register) */
    console_printf("      Configuring TCR_EL1...\n");
    uint64_t tcr = 0;
    tcr |= (16ULL << 0);   /* T0SZ = 16 (48-bit VA for TTBR0) */
    tcr |= (0ULL << 6);    /* Reserved */
    tcr |= (0ULL << 7);    /* EPD0 = 0 (enable TTBR0 walks) */
    tcr |= (3ULL << 8);    /* IRGN0 = 3 (inner write-back cacheable) */
    tcr |= (3ULL << 10);   /* ORGN0 = 3 (outer write-back cacheable) */
    tcr |= (3ULL << 12);   /* SH0 = 3 (inner shareable) */
    tcr |= (0ULL << 14);   /* TG0 = 0 (4KB granule for TTBR0) - FIXED! */
    tcr |= (16ULL << 16);  /* T1SZ = 16 (48-bit VA for TTBR1) */
    tcr |= (0ULL << 22);   /* A1 = 0 (TTBR0 defines ASID) */
    tcr |= (1ULL << 23);   /* EPD1 = 1 (disable TTBR1 walks for now) */
    tcr |= (3ULL << 24);   /* IRGN1 = 3 */
    tcr |= (3ULL << 26);   /* ORGN1 = 3 */
    tcr |= (3ULL << 28);   /* SH1 = 3 */
    tcr |= (0ULL << 30);   /* TG1 = 0 (4KB granule for TTBR1) - FIXED! */
    tcr |= (1ULL << 32);   /* IPS = 1 (36-bit PA space, 64GB) - FIXED! Was 25! */
    __asm__ volatile("msr tcr_el1, %0" :: "r"(tcr));
    console_printf("      TCR_EL1 configured\n");

    /* Set TTBR0_EL1 to kernel page table */
    console_printf("      Setting TTBR0_EL1 to 0x%llx...\n", (uint64_t)kernel_pgd);
    __asm__ volatile("msr ttbr0_el1, %0" :: "r"((uint64_t)kernel_pgd));
    console_printf("      TTBR0_EL1 set\n");

    /* Invalidate all TLB entries */
    console_printf("      Invalidating TLB...\n");
    __asm__ volatile("tlbi vmalle1");
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");
    console_printf("      TLB invalidated\n");

    /* Enable MMU: Set SCTLR_EL1.M bit */
    console_printf("      Enabling MMU (without caches for now)...\n");
    uint64_t sctlr;
    __asm__ volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
    sctlr |= (1 << 0);  /* M bit - Enable MMU */
    /* Don't enable caches yet - test MMU first */
    // sctlr |= (1 << 2);  /* C bit - Enable data cache */
    // sctlr |= (1 << 12); /* I bit - Enable instruction cache */
    __asm__ volatile("msr sctlr_el1, %0" :: "r"(sctlr));
    __asm__ volatile("isb");

    console_printf("      MMU enabled with 4-level page tables\n");
}
