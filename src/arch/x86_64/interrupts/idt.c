#include <kernel/types.h>
#include <kernel/console.h>
#include <kernel/string.h>
#include <arch/interrupts.h>

/* IDT Entry */
struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

/* IDT Pointer */
struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr idtp;

/* IRQ handlers table */
static irq_handler_t irq_handlers[16] = {0};

/* External ASM functions */
extern void idt_load(uint64_t);
extern void* isr_stub_table[];

/* PIC ports */
#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

/* I/O port functions */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void idt_set_gate(uint8_t num, uint64_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_mid = (base >> 16) & 0xFFFF;
    idt[num].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].selector = selector;
    idt[num].ist = 0;
    idt[num].flags = flags;
    idt[num].reserved = 0;
}

/* Remap PIC */
static void pic_remap(void) {
    /* Start initialization */
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    /* Set vector offsets */
    outb(PIC1_DATA, 0x20);  /* IRQ 0-7  mapped to 0x20-0x27 */
    outb(PIC2_DATA, 0x28);  /* IRQ 8-15 mapped to 0x28-0x2F */

    /* Setup cascade */
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    /* 8086 mode */
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    /* Mask all interrupts except cascade */
    outb(PIC1_DATA, 0xFD);  /* Enable IRQ1 (keyboard) */
    outb(PIC2_DATA, 0xFF);
}

void interrupts_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint64_t)&idt;

    memset(&idt, 0, sizeof(struct idt_entry) * 256);

    /* Install ISRs */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }

    /* Remap PIC */
    pic_remap();

    /* Load IDT */
    idt_load((uint64_t)&idtp);
}

void interrupts_enable(void) {
    __asm__ volatile("sti");
}

void interrupts_disable(void) {
    __asm__ volatile("cli");
}

void irq_install_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_uninstall_handler(uint8_t irq) {
    if (irq < 16) {
        irq_handlers[irq] = 0;
    }
}

/* IRQ handler called from ASM */
void irq_handler(uint64_t irq_num) {
    /* Send EOI */
    if (irq_num >= 8) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);

    /* Call handler */
    if (irq_handlers[irq_num]) {
        irq_handlers[irq_num]();
    }
}

/* Exception handler */
void isr_handler(uint64_t isr_num) {
    console_printf("\n*** EXCEPTION: %d ***\n", isr_num);

    const char* exceptions[] = {
        "Division By Zero", "Debug", "Non Maskable Interrupt", "Breakpoint",
        "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
        "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
        "Stack-Segment Fault", "General Protection Fault", "Page Fault", "Reserved",
        "x87 Floating-Point Exception", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception"
    };

    if (isr_num < 20) {
        console_printf("Exception: %s\n", exceptions[isr_num]);
    }

    console_printf("System halted.\n");
    while(1) {
        __asm__ volatile("hlt");
    }
}
