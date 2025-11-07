#include <kernel/console.h>
#include <kernel/types.h>

/* QEMU virt machine PL011 UART base address */
#define UART0_BASE 0x09000000

/* UART registers */
#define UART0_DR     (*(volatile uint32_t*)(UART0_BASE + 0x00))  /* Data Register */
#define UART0_FR     (*(volatile uint32_t*)(UART0_BASE + 0x18))  /* Flag Register */
#define UART0_IBRD   (*(volatile uint32_t*)(UART0_BASE + 0x24))  /* Integer Baud Rate Divisor */
#define UART0_FBRD   (*(volatile uint32_t*)(UART0_BASE + 0x28))  /* Fractional Baud Rate Divisor */
#define UART0_LCRH   (*(volatile uint32_t*)(UART0_BASE + 0x2C))  /* Line Control Register */
#define UART0_CR     (*(volatile uint32_t*)(UART0_BASE + 0x30))  /* Control Register */
#define UART0_IMSC   (*(volatile uint32_t*)(UART0_BASE + 0x38))  /* Interrupt Mask Set/Clear */
#define UART0_ICR    (*(volatile uint32_t*)(UART0_BASE + 0x44))  /* Interrupt Clear Register */

/* Flag register bits */
#define UART_FR_TXFF (1 << 5)  /* Transmit FIFO full */
#define UART_FR_RXFE (1 << 4)  /* Receive FIFO empty */

/* Control register bits */
#define UART_CR_UARTEN (1 << 0)  /* UART enable */
#define UART_CR_TXE    (1 << 8)  /* Transmit enable */
#define UART_CR_RXE    (1 << 9)  /* Receive enable */

/* Line control register bits */
#define UART_LCRH_WLEN_8BIT (3 << 5)  /* 8-bit word length */
#define UART_LCRH_FEN       (1 << 4)  /* Enable FIFOs */

void console_init(void) {
    /* Disable UART */
    UART0_CR = 0;

    /* Clear all interrupts */
    UART0_ICR = 0x7FF;

    /* Set baud rate to 115200 */
    /* UART clock is 24MHz on QEMU virt */
    /* Divisor = UART_CLK / (16 * Baud) = 24000000 / (16 * 115200) = 13.02 */
    UART0_IBRD = 13;
    UART0_FBRD = 1;  /* Fractional part: 0.02 * 64 = 1 */

    /* Set 8-bit, no parity, 1 stop bit, enable FIFOs */
    UART0_LCRH = UART_LCRH_WLEN_8BIT | UART_LCRH_FEN;

    /* Disable interrupts for now */
    UART0_IMSC = 0;

    /* Enable UART, TX, and RX */
    UART0_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void console_putchar(char c) {
    /* Wait until TX FIFO is not full */
    while (UART0_FR & UART_FR_TXFF);

    /* Write character */
    UART0_DR = (uint32_t)c;

    /* Convert \n to \r\n for proper terminal output */
    if (c == '\n') {
        while (UART0_FR & UART_FR_TXFF);
        UART0_DR = '\r';
    }
}

char uart_getchar(void) {
    /* Wait until RX FIFO is not empty */
    while (UART0_FR & UART_FR_RXFE);

    /* Read character */
    return (char)(UART0_DR & 0xFF);
}

bool uart_has_data(void) {
    return !(UART0_FR & UART_FR_RXFE);
}

void console_clear(void) {
    /* Send ANSI escape code to clear screen */
    console_write("\033[2J\033[H");
}
