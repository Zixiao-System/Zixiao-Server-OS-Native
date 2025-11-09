#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <kernel/types.h>

/* Architecture-specific register dump structure */
typedef struct {
    uint64_t regs[32];  /* General purpose registers */
    uint64_t pc;        /* Program counter */
    uint64_t sp;        /* Stack pointer */
    uint64_t lr;        /* Link register (ARM64) / Return address (x86_64) */
    uint64_t flags;     /* Processor flags */
} panic_regs_t;

/* Kernel panic function - never returns */
void kernel_panic(const char* message, panic_regs_t* regs) __attribute__((noreturn));

/* Architecture-specific panic implementation */
void arch_panic_dump_regs(panic_regs_t* regs);
void arch_halt(void) __attribute__((noreturn));

#endif /* KERNEL_PANIC_H */
