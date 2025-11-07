#include <kernel/types.h>
#include <kernel/string.h>

/* GDT Entry */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

/* GDT Pointer */
struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

/* GDT entries */
static struct gdt_entry gdt[5];
static struct gdt_ptr gp;

/* External ASM function to load GDT */
extern void gdt_flush(uint64_t);

static void gdt_set_gate(int num, uint64_t base, uint64_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // Code segment (64-bit)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xAF); // Data segment (64-bit)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF); // User code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xAF); // User data

    gdt_flush((uint64_t)&gp);
}
