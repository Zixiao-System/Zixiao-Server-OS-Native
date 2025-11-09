#include <kernel/panic.h>
#include <kernel/console.h>
#include <kernel/types.h>

/* Dump x86_64 registers */
void arch_panic_dump_regs(panic_regs_t* regs) {
    if (!regs) {
        console_printf("No register information available.\n");
        return;
    }

    console_printf("Instruction Pointer (RIP): 0x%016llx\n", regs->pc);
    console_printf("Stack Pointer (RSP):       0x%016llx\n", regs->sp);
    console_printf("Base Pointer (RBP):        0x%016llx\n", regs->regs[5]); /* RBP */
    console_printf("RFLAGS:                    0x%016llx\n", regs->flags);
    console_printf("\n");

    console_printf("General Purpose Registers:\n");
    const char* reg_names[] = {
        "RAX", "RBX", "RCX", "RDX", "RSI", "RDI", "RBP", "RSP",
        "R8",  "R9",  "R10", "R11", "R12", "R13", "R14", "R15"
    };

    for (int i = 0; i < 16; i++) {
        console_printf("  %s: 0x%016llx", reg_names[i], regs->regs[i]);
        if ((i + 1) % 2 == 0) {
            console_printf("\n");
        } else {
            console_printf("  ");
        }
    }
    console_printf("\n");

    /* Read control registers */
    uint64_t cr0, cr2, cr3, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));

    console_printf("Control Registers:\n");
    console_printf("  CR0: 0x%016llx (PE=%lld, PG=%lld)\n", cr0, cr0 & 1, (cr0 >> 31) & 1);
    console_printf("  CR2: 0x%016llx (Page Fault Address)\n", cr2);
    console_printf("  CR3: 0x%016llx (Page Directory Base)\n", cr3);
    console_printf("  CR4: 0x%016llx\n", cr4);
}

/* Halt the x86_64 system */
void arch_halt(void) {
    /* Disable interrupts */
    __asm__ volatile("cli");

    /* Infinite halt loop */
    while (1) {
        __asm__ volatile("hlt");
    }
}
