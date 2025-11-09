#include <kernel/panic.h>
#include <kernel/console.h>
#include <kernel/types.h>

/* Dump ARM64 registers */
void arch_panic_dump_regs(panic_regs_t* regs) {
    if (!regs) {
        console_printf("No register information available.\n");
        return;
    }

    console_printf("Program Counter (PC):  0x%016llx\n", regs->pc);
    console_printf("Stack Pointer (SP):    0x%016llx\n", regs->sp);
    console_printf("Link Register (LR):    0x%016llx\n", regs->lr);
    console_printf("Processor State (PSTATE): 0x%016llx\n", regs->flags);
    console_printf("\n");

    /* Print general purpose registers in groups of 2 */
    console_printf("General Purpose Registers:\n");
    for (int i = 0; i < 31; i++) {
        if (i < 10) {
            console_printf("  X%d:  0x%016llx", i, regs->regs[i]);
        } else {
            console_printf("  X%d: 0x%016llx", i, regs->regs[i]);
        }
        if ((i + 1) % 2 == 0) {
            console_printf("\n");
        } else {
            console_printf("  ");
        }
    }
    console_printf("\n");

    /* Print special system registers */
    uint64_t currentel, sp_el0, elr_el1, esr_el1, far_el1;

    __asm__ volatile("mrs %0, CurrentEL" : "=r"(currentel));
    __asm__ volatile("mrs %0, SP_EL0" : "=r"(sp_el0));
    __asm__ volatile("mrs %0, ELR_EL1" : "=r"(elr_el1));
    __asm__ volatile("mrs %0, ESR_EL1" : "=r"(esr_el1));
    __asm__ volatile("mrs %0, FAR_EL1" : "=r"(far_el1));

    console_printf("System Registers:\n");
    console_printf("  CurrentEL:  0x%016llx (EL%lld)\n", currentel, (currentel >> 2) & 0x3);
    console_printf("  SP_EL0:     0x%016llx\n", sp_el0);
    console_printf("  ELR_EL1:    0x%016llx (Exception Return Address)\n", elr_el1);
    console_printf("  ESR_EL1:    0x%016llx (Exception Syndrome)\n", esr_el1);
    console_printf("  FAR_EL1:    0x%016llx (Fault Address)\n", far_el1);

    /* Decode ESR_EL1 */
    uint32_t ec = (esr_el1 >> 26) & 0x3F;  /* Exception Class */
    uint32_t iss = esr_el1 & 0x1FFFFFF;     /* Instruction Specific Syndrome */

    console_printf("\n  Exception Class (EC): 0x%x ", ec);
    switch (ec) {
        case 0x00: console_printf("(Unknown reason)\n"); break;
        case 0x01: console_printf("(Trapped WFI/WFE)\n"); break;
        case 0x07: console_printf("(Access to SVE/SIMD/FP)\n"); break;
        case 0x15: console_printf("(SVC instruction execution in AArch64)\n"); break;
        case 0x20: console_printf("(Instruction Abort from lower EL)\n"); break;
        case 0x21: console_printf("(Instruction Abort from same EL)\n"); break;
        case 0x22: console_printf("(PC alignment fault)\n"); break;
        case 0x24: console_printf("(Data Abort from lower EL)\n"); break;
        case 0x25: console_printf("(Data Abort from same EL)\n"); break;
        case 0x26: console_printf("(SP alignment fault)\n"); break;
        case 0x30: console_printf("(Breakpoint from lower EL)\n"); break;
        case 0x31: console_printf("(Breakpoint from same EL)\n"); break;
        default: console_printf("(Other)\n"); break;
    }
    console_printf("  ISS: 0x%x\n", iss);
}

/* Halt the ARM64 system */
void arch_halt(void) {
    /* Disable all interrupts */
    __asm__ volatile("msr daifset, #0xF" ::: "memory");

    /* Infinite loop with WFI (Wait For Interrupt) */
    while (1) {
        __asm__ volatile("wfi");
    }
}
