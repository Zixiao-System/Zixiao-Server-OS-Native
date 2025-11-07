#include <kernel/console.h>
#include <kernel/types.h>
#include <kernel/string.h>

// Variable arguments support
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

static void print_int(int64_t num, int base, int is_signed) {
    char buffer[32];
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        console_putchar('0');
        return;
    }

    if (is_signed && num < 0) {
        is_negative = 1;
        num = -num;
    }

    uint64_t unum = (uint64_t)num;

    while (unum != 0) {
        int rem = unum % base;
        buffer[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        unum = unum / base;
    }

    if (is_negative)
        console_putchar('-');

    while (i > 0)
        console_putchar(buffer[--i]);
}

void console_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd':
                case 'i': {
                    int val = va_arg(args, int);
                    print_int(val, 10, 1);
                    break;
                }
                case 'u': {
                    unsigned int val = va_arg(args, unsigned int);
                    print_int(val, 10, 0);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    console_write("0x");
                    print_int(val, 16, 0);
                    break;
                }
                case 'p': {
                    uint64_t val = va_arg(args, uint64_t);
                    console_write("0x");
                    print_int(val, 16, 0);
                    break;
                }
                case 's': {
                    const char* str = va_arg(args, const char*);
                    console_write(str ? str : "(null)");
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    console_putchar(c);
                    break;
                }
                case '%':
                    console_putchar('%');
                    break;
                default:
                    console_putchar('%');
                    console_putchar(*fmt);
                    break;
            }
        } else {
            console_putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}

void console_write(const char* str) {
    while (*str) {
        console_putchar(*str++);
    }
}
