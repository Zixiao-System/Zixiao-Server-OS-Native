#include <kernel/console.h>
#include <kernel/types.h>
#include <kernel/string.h>

// Variable arguments support
typedef __builtin_va_list va_list;
#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)

static void print_int(int64_t num, int base, int is_signed, int width, char pad_char) {
    char buffer[32];
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        /* Handle zero padding */
        for (int j = 1; j < width; j++) {
            console_putchar(pad_char);
        }
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

    /* Add padding */
    int total_chars = i + (is_negative ? 1 : 0);
    for (int j = total_chars; j < width; j++) {
        console_putchar(pad_char);
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

            /* Check for padding character (0 or space) */
            char pad_char = ' ';
            if (*fmt == '0') {
                pad_char = '0';
                fmt++;
            }

            /* Parse width */
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            /* Check for 'll' prefix (long long) */
            int is_long_long = 0;
            if (*fmt == 'l' && *(fmt + 1) == 'l') {
                is_long_long = 1;
                fmt += 2;
            }

            switch (*fmt) {
                case 'd':
                case 'i': {
                    if (is_long_long) {
                        int64_t val = va_arg(args, int64_t);
                        print_int(val, 10, 1, width, pad_char);
                    } else {
                        int val = va_arg(args, int);
                        print_int(val, 10, 1, width, pad_char);
                    }
                    break;
                }
                case 'u': {
                    if (is_long_long) {
                        uint64_t val = va_arg(args, uint64_t);
                        print_int(val, 10, 0, width, pad_char);
                    } else {
                        unsigned int val = va_arg(args, unsigned int);
                        print_int(val, 10, 0, width, pad_char);
                    }
                    break;
                }
                case 'x': {
                    if (is_long_long) {
                        uint64_t val = va_arg(args, uint64_t);
                        print_int(val, 16, 0, width, pad_char);
                    } else {
                        unsigned int val = va_arg(args, unsigned int);
                        print_int(val, 16, 0, width, pad_char);
                    }
                    break;
                }
                case 'p': {
                    uint64_t val = va_arg(args, uint64_t);
                    console_write("0x");
                    print_int(val, 16, 0, 0, ' ');
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
                    if (is_long_long) {
                        console_write("ll");
                    }
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
