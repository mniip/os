#ifndef VGA_IO_H
#define VGA_IO_H

extern void vga_set_color(int, int);
extern int vga_get_fg();
extern int vga_get_bg();
extern void vga_printf(char const *, ...);

#endif
