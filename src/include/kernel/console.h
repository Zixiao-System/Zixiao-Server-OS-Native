#ifndef ZIXIAO_CONSOLE_H
#define ZIXIAO_CONSOLE_H

#include <kernel/types.h>

void console_init(void);
void console_putchar(char c);
void console_write(const char* str);
void console_printf(const char* fmt, ...);
void console_clear(void);

#endif // ZIXIAO_CONSOLE_H