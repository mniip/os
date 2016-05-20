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

void vga_printf(char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	while(*fmt)
	{
		if(*fmt == '%')
		{
			fmt++;
			if(*fmt)
			{
				int width = -1;
				if(*fmt >= '0' && *fmt <= '9')
				{
					width = 0;
					while(*fmt >= '0' && *fmt <= '9')
					{
						width = width * 10 + (*fmt - '0');
						fmt++;
					}
				}
				if(*fmt == 'l')
				{
					fmt++;
					if(*fmt == 'x')
					{
						uint64_t x = va_arg(arg, uint64_t), tmp = x;
						if(width == -1)
						{
							width = 0;
							while(tmp)
							{
								tmp >>= 4;
								width++;
							}
							if(!width)
								width++;
						}
						char str[width];
						int i;
						for(i = width - 1; i >= 0; i--)
						{
							int digit = x & 0xF;
							x >>= 4;
							str[i] = digit < 10 ? digit + '0' : digit - 10 + 'A';
						}
						for(i = 0; i < width; i++)
							vga_put_char(str[i]);
						fmt++;
					}
					else if(*fmt == 'o')
					{
						uint64_t x = va_arg(arg, uint64_t), tmp = x;
						if(width == -1)
						{
							width = 0;
							while(tmp)
							{
								tmp >>= 3;
								width++;
							}
							if(!width)
								width++;
						}
						char str[width];
						int i;
						for(i = width - 1; i >= 0; i--)
						{
							int digit = x & 0x7;
							x >>= 3;
							str[i] = digit + '0';
						}
						for(i = 0; i < width; i++)
							vga_put_char(str[i]);
						fmt++;
					}
				}
				else
				{
					if(*fmt == 'x')
					{
						uint32_t x = va_arg(arg, uint32_t), tmp = x;
						if(width == -1)
						{
							width = 0;
							while(tmp)
							{
								tmp >>= 4;
								width++;
							}
							if(!width)
								width++;
						}
						char str[width];
						int i;
						for(i = width - 1; i >= 0; i--)
						{
							int digit = x & 0xF;
							x >>= 4;
							str[i] = digit < 10 ? digit + '0' : digit - 10 + 'A';
						}
						for(i = 0; i < width; i++)
							vga_put_char(str[i]);
						fmt++;
					}
					else if(*fmt == 'o')
					{
						uint32_t x = va_arg(arg, uint32_t), tmp = x;
						if(width == -1)
						{
							width = 0;
							while(tmp)
							{
								tmp >>= 3;
								width++;
							}
							if(!width)
								width++;
						}
						char str[width];
						int i;
						for(i = width - 1; i >= 0; i--)
						{
							int digit = x & 0x7;
							x >>= 3;
							str[i] = digit + '0';
						}
						for(i = 0; i < width; i++)
							vga_put_char(str[i]);
						fmt++;
					}
					else if(*fmt == 'u')
					{
						uint32_t x = va_arg(arg, uint32_t), tmp = x;
						if(width == -1)
						{
							width = 0;
							while(tmp)
							{
								tmp /= 10;
								width++;
							}
							if(!width)
								width++;
						}
						char str[width];
						int i;
						for(i = width - 1; i >= 0; i--)
						{
							int digit = x % 10;
							x /= 10;
							str[i] = digit + '0';
						}
						for(i = 0; i < width; i++)
							vga_put_char(str[i]);
						fmt++;
					}
					else if(*fmt == 's')
					{
						char const *x = va_arg(arg, char const *);
						while(*x)
						{
							vga_put_char(*x);
							x++;
						}
						fmt++;
					}
				}
			}
		}
		else
		{
			vga_put_char(*fmt);
			fmt++;
		}
	}
	vga_sync_cursor();
}
