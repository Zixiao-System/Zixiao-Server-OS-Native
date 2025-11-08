/**
 * x86_64 MMU and Page Table Management
 * Implements 4-level page tables (PML4→PDPT→PD→PT) for 48-bit virtual addresses with 4KB pages
 */

#include <arch/x86_64_mmu.h>
#include <kernel/mm.h>
#include <kernel/string.h>
#include <kernel/console.h>

/* Kernel page table (shared across all processes) */
static page_table_t kernel_pml4 = NULL;

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

    /* Check if entry already exists and is present */
    if (entry & PTE_PRESENT) {
        if (!is_leaf) {
            /* Return existing table (extract physical address) */
            return (uint64_t*)(entry & PTE_ADDR_MASK);
        } else {
            /* Leaf level - entry already mapped */
            return table;
        }
    }

    /* Create new table if not leaf level */
    if (!is_leaf) {
        uint64_t* new_table = alloc_page_table();
        if (new_table == NULL) {
            return NULL;
        }

        /* Install table descriptor with PRESENT and WRITE flags */
        table[index] = ((uint64_t)new_table & PTE_ADDR_MASK) | PTE_PRESENT | PTE_WRITE;
        return new_table;
    }

    /* Leaf level - caller will install actual mapping */
    return table;
}

/**
 * Map a virtual page to a physical page
 */
int x86_64_map_page(page_table_t pml4, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags)
{
    /* Ensure addresses are page-aligned */
    virt_addr &= ~(PAGE_SIZE - 1);
    phys_addr &= ~(PAGE_SIZE - 1);

    /* Debug: Only print for special addresses to avoid spam */
    if (virt_addr == 0xB8000) {
        console_printf("        [DEBUG] Mapping VGA 0x%llx -> 0x%llx\n", virt_addr, phys_addr);
    }

    /* Walk through page table levels: PML4 → PDPT → PD → PT */
    uint64_t* pdpt = get_or_create_table(pml4, PML4_INDEX(virt_addr), 0);
    if (pdpt == NULL) return -1;

    uint64_t* pd = get_or_create_table(pdpt, PDPT_INDEX(virt_addr), 0);
    if (pd == NULL) return -1;

    uint64_t* pt = get_or_create_table(pd, PD_INDEX(virt_addr), 0);
    if (pt == NULL) return -1;

    /* Install leaf PTE */
    uint64_t pt_index = PT_INDEX(virt_addr);

    /* Check if this is device memory (use NOCACHE flag for device memory) */
    uint64_t pte_flags = flags & ~PTE_DEVICE;  /* Clear custom flag */
    if (flags & PTE_DEVICE) {
        pte_flags |= PTE_NOCACHE | PTE_WRITETHROUGH;  /* Device memory: uncacheable */
    }

    /* Build PTE: physical address + flags + PRESENT */
    uint64_t pte = (phys_addr & PTE_ADDR_MASK) | pte_flags | PTE_PRESENT | PTE_ACCESSED;
    pt[pt_index] = pte;

    return 0;
}

/**
 * Unmap a virtual page
 */
void x86_64_unmap_page(page_table_t pml4, uint64_t virt_addr)
{
    virt_addr &= ~(PAGE_SIZE - 1);

    /* Walk page tables to find PTE */
    uint64_t entry = pml4[PML4_INDEX(virt_addr)];
    if (!(entry & PTE_PRESENT)) return;
    uint64_t* pdpt = (uint64_t*)(entry & PTE_ADDR_MASK);

    entry = pdpt[PDPT_INDEX(virt_addr)];
    if (!(entry & PTE_PRESENT)) return;
    uint64_t* pd = (uint64_t*)(entry & PTE_ADDR_MASK);

    entry = pd[PD_INDEX(virt_addr)];
    if (!(entry & PTE_PRESENT)) return;
    uint64_t* pt = (uint64_t*)(entry & PTE_ADDR_MASK);

    /* Clear PTE */
    pt[PT_INDEX(virt_addr)] = 0;

    /* Invalidate TLB for this address */
    __asm__ volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
}

/**
 * Create a new empty page table
 */
page_table_t x86_64_create_page_table(void)
{
    return alloc_page_table();
}

/**
 * Switch to a different page table (load CR3)
 */
void x86_64_switch_page_table(page_table_t pml4)
{
    uint64_t cr3 = (uint64_t)pml4;

    /* Load CR3 (also flushes TLB) */
    __asm__ volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
}

/**
 * Get current page table base from CR3
 */
uint64_t x86_64_get_current_page_table(void)
{
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3 & PTE_ADDR_MASK;  /* Mask off control bits */
}

/**
 * Initialize x86_64 MMU
 */
void x86_64_mmu_init(void)
{
    /* Create kernel page table */
    kernel_pml4 = x86_64_create_page_table();
    if (kernel_pml4 == NULL) {
        console_printf("ERROR: Failed to create kernel page table\n");
        while(1);
    }

    /* Identity map kernel region (8 MB starting at 1MB)
     * x86_64 kernel loads at 1MB (0x100000), map up to 0x900000 (9MB total)
     */
    console_printf("      Identity mapping kernel region (8 MB)...\n");

    uint64_t map_start = 0x100000;   /* 1MB - kernel start */
    uint64_t map_end = 0x900000;     /* 9MB - kernel + heap + stack */
    uint64_t pages_mapped = 0;

    for (uint64_t addr = map_start; addr < map_end; addr += PAGE_SIZE) {
        if (x86_64_map_page(kernel_pml4, addr, addr, PTE_WRITE) != 0) {
            console_printf("ERROR: Failed to map 0x%llx\n", addr);
            while(1);
        }
        pages_mapped++;

        /* Print progress every 512 pages (2MB) */
        if (pages_mapped % 512 == 0) {
            console_printf("      Mapped %llu MB...\n", (pages_mapped * PAGE_SIZE) / (1024 * 1024));
        }
    }
    console_printf("      Mapped %llu pages (%llu MB)\n",
                   pages_mapped, (pages_mapped * PAGE_SIZE) / (1024 * 1024));

    /* Map VGA text buffer (0xB8000) - CRITICAL for console after MMU enable! */
    console_printf("      Mapping VGA buffer (0xB8000)...\n");
    if (x86_64_map_page(kernel_pml4, 0xB8000, 0xB8000, PTE_WRITE | PTE_DEVICE) != 0) {
        console_printf("ERROR: Failed to map VGA\n");
        while(1);
    }
    console_printf("      VGA mapped\n");

    /* Set CR3 to kernel page table */
    console_printf("      Setting CR3 to 0x%llx...\n", (uint64_t)kernel_pml4);
    __asm__ volatile("mov %0, %%cr3" :: "r"((uint64_t)kernel_pml4) : "memory");
    console_printf("      CR3 set\n");

    /* CR0.PG should already be enabled by boot.S, but verify and ensure it's set */
    console_printf("      Verifying paging is enabled...\n");
    uint64_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    if (!(cr0 & (1ULL << 31))) {
        console_printf("      WARNING: Paging not enabled, enabling now...\n");
        cr0 |= (1ULL << 31);  /* CR0.PG - Enable paging */
        cr0 |= (1ULL << 16);  /* CR0.WP - Write protect kernel pages */
        __asm__ volatile("mov %0, %%cr0" :: "r"(cr0) : "memory");
    }

    console_printf("      MMU enabled with 4-level page tables (PML4→PDPT→PD→PT)\n");
}
