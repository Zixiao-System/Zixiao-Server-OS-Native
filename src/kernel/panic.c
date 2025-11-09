#include <kernel/panic.h>
#include <kernel/console.h>
#include <kernel/string.h>

/* Console width for centering text */
#define CONSOLE_WIDTH 80

/* Mysterious quotes from Zixiao mythology */
static const char* mysterious_quotes[] = {
    "神识不及，涣散溃败亦是常理。",
    "兵未厉，驮未秣，万事未备，东风难至。",
    "若遭变故，恕不赔偿。",
    "闲人勿入。",
    "天险多枯骨，向来如是。",
    "可叹，可叹，汝不及吾。",
    "上山易，下山难。"
};

#define QUOTES_COUNT (sizeof(mysterious_quotes) / sizeof(mysterious_quotes[0]))

/* Simple pseudo-random number generator (Linear Congruential Generator) */
static uint64_t panic_rand_seed = 0x5D1A0C0DE;  /* Default seed */

static uint32_t panic_rand(void) {
    /* LCG parameters from Numerical Recipes */
    panic_rand_seed = panic_rand_seed * 1664525ULL + 1013904223ULL;
    return (uint32_t)(panic_rand_seed >> 32);
}

/* Print centered text */
static void print_centered(const char* text) {
    size_t len = strlen(text);
    if (len >= CONSOLE_WIDTH) {
        console_printf("%s\n", text);
        return;
    }

    int padding = (CONSOLE_WIDTH - len) / 2;
    for (int i = 0; i < padding; i++) {
        console_printf(" ");
    }
    console_printf("%s\n", text);
}

/* Print a separator line */
static void print_separator(void) {
    for (int i = 0; i < CONSOLE_WIDTH; i++) {
        console_printf("=");
    }
    console_printf("\n");
}

/* Main kernel panic function */
void kernel_panic(const char* message, panic_regs_t* regs) {
    /* Disable interrupts to prevent further chaos */
    /* Architecture-specific interrupt disable is done in arch_halt() */

    console_printf("\n\n");
    print_separator();
    console_printf("\n");

    /* Print main panic message */
    console_printf("KERNEL PANIC!\n");
    console_printf("\n");

    /* Print the iconic phrases in centered format */
    print_centered("\u4e00 \u8d25 \u6d82 \u5730");  /* 一 败 涂 地 */
    console_printf("\n");
    print_centered("\u6e38\u56ed\u60ca\u68a6");  /* 游园惊梦 */
    console_printf("\n");

    /* Select and print a random mysterious quote */
    uint32_t quote_index = panic_rand() % QUOTES_COUNT;
    print_centered(mysterious_quotes[quote_index]);
    console_printf("\n");

    print_separator();
    console_printf("\n");

    /* Print the panic message */
    if (message) {
        console_printf("Panic message: %s\n", message);
        console_printf("\n");
    }

    /* Print register dump if available */
    if (regs) {
        console_printf("=== Register Dump ===\n");
        arch_panic_dump_regs(regs);
        console_printf("\n");
    }

    /* Print stack trace hint */
    console_printf("=== Stack Trace ===\n");
    console_printf("Stack Pointer (SP): 0x%llx\n", regs ? regs->sp : 0);
    console_printf("Program Counter (PC): 0x%llx\n", regs ? regs->pc : 0);
    console_printf("Link Register (LR): 0x%llx\n", regs ? regs->lr : 0);
    console_printf("\n");

    /* Print memory around stack pointer for debugging */
    if (regs && regs->sp != 0) {
        console_printf("=== Stack Contents (top 8 entries) ===\n");
        uint64_t* stack = (uint64_t*)regs->sp;
        for (int i = 0; i < 8; i++) {
            console_printf("[SP+%d]: 0x%llx\n", i * 8, stack[i]);
        }
        console_printf("\n");
    }

    print_separator();
    console_printf("\n");
    console_printf("System halted. Please reboot.\n");
    console_printf("\n");

    /* Architecture-specific halt */
    arch_halt();
}
