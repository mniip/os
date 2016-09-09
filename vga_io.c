#include <stdint.h>
#include <stdarg.h>
#include "asm.h"

static int cursor = 0;
static uint8_t color = 0x07;

#define VGA_RAM ((uint16_t *)0xB8000)
#define VGA_LINES 25
#define VGA_COLUMNS 80

void vga_auto_scroll()
{
	while(cursor >= VGA_LINES * VGA_COLUMNS)
	{
		int i;
		for(i = 0; i < (VGA_LINES - 1) * VGA_COLUMNS; i++)
			VGA_RAM[i] = VGA_RAM[i + VGA_COLUMNS];
		for(i = 0; i < VGA_COLUMNS; i++)
			VGA_RAM[(VGA_LINES - 1) * VGA_COLUMNS + i] = (uint16_t)' ' | (uint16_t)color << 8;
		cursor -= VGA_COLUMNS;
	}
}

void vga_put_char(char c)
{
	if(c == '\n')
	{
		cursor = (cursor / VGA_COLUMNS + 1) * VGA_COLUMNS;
		vga_auto_scroll();
	}
	else if(c == '\b')
	{
		cursor--;
	}
	else
	{
		VGA_RAM[cursor] = (uint16_t)c | (uint16_t)color << 8;
		cursor++;
		vga_auto_scroll();
	}
}

void vga_sync_cursor()
{
	outb(0x3D4, 0x0F);
	outb(0x3D5, cursor & 0xFF);
	outb(0x3D4, 0x0E);
	outb(0x3D5, cursor >> 8);
}

void vga_reset()
{
	int i;
	for(i = 0; i < VGA_LINES * VGA_COLUMNS; i++)
		VGA_RAM[i] = (uint16_t)' ' | (uint16_t)color << 8;
	cursor = 0;
	vga_sync_cursor();
}

void vga_set_color(int fg, int bg)
{
	color = bg << 4 | (fg & 0x0F);
}

int vga_get_bg()
{
	return color >> 4;
}

int vga_get_fg()
{
	return color & 0x0F;
}
