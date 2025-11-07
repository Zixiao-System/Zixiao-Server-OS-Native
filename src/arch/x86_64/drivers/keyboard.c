#include <kernel/types.h>
#include <kernel/console.h>
#include <arch/interrupts.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* US QWERTY scancode to ASCII table */
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static void keyboard_irq_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    /* Only handle key press (not release) */
    if (scancode & 0x80) {
        return;  /* Key release */
    }

    /* Convert scancode to ASCII */
    if (scancode < sizeof(scancode_to_ascii)) {
        char c = scancode_to_ascii[scancode];
        if (c) {
            console_putchar(c);
        }
    }
}

void keyboard_init(void) {
    /* Install keyboard IRQ handler */
    irq_install_handler(1, keyboard_irq_handler);
    console_printf("Keyboard driver initialized\n");
}
