#ifndef VGA_IO_H
#define VGA_IO_H

#include <stdarg.h>

extern void vga_set_color(int, int);
extern int vga_get_fg();
extern int vga_get_bg();
extern void vga_put_char(char);
extern void vga_sync_cursor();
extern void vga_reset();

extern int vga_printf(char const *, ...);
extern int vga_vprintf(char const *, va_list);

#endif
