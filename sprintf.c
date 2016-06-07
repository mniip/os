#include <stdint.h>
#include <stddef.h>
#include "vga_io.h"
#include "sprintf.h"

size_t sprintf(char *buf, char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	size_t ret = vsprintf(buf, fmt, arg);
	va_end(arg);
	return ret;
}

void vga_printf(char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	vga_vprintf(fmt, arg);
	va_end(arg);
}

enum {
	DEFAULT,
	HALF_HALF,
	HALF,
	LONG,
	LONG_LONG,
	INTMAX_T,
	SIZE_T,
	PTRDIFF_T,
};

enum {
	ALTERNATE = 0x1,
	ZERO_PAD = 0x2,
	LEFT_ADJUST = 0x4,
	BLANK = 0x8,
	SIGN = 0x10,
	UPPERCASE = 0x20,
};

static inline long long unsigned int mul8(long long unsigned int value)
{
	return value << 3;
}

static inline long long unsigned int mul10(long long unsigned int value)
{
	return (value << 3) + (value << 1);
}

static inline long long unsigned int mul16(long long unsigned int value)
{
	return value << 4;
}

static inline long long unsigned int div8(long long unsigned int value)
{
	return value >> 3;
}

static inline long long unsigned int div10(long long unsigned int value)
{
	long long unsigned int t = (value >> 1) + (value >> 2);
	t += t >> 4;
	t += t >> 8;
	t += t >> 16;
	t += t >> 32;
	t >>= 3;
	t += (value - (t << 3) - (t << 1)) >= 10;
	return t;
}

static inline long long unsigned int div16(long long unsigned int value)
{
	return value >> 4;
}

static char *write_alternate(char *buf, int flags, int base)
{
	if(flags & ALTERNATE)
	{
		if(base == 8)
			*(buf++) = '0';
		else if(base == 16)
		{
			*(buf++) = '0';
			*(buf++) = 'x';
		}
	}
	return buf;
}

static char *write_pad(char *buf, int flags, int size, int pre)
{
	int i;
	if(!(flags & LEFT_ADJUST) != !pre)
		for(i = 0; i < size; i++)
			*(buf++) = pre && (flags & ZERO_PAD) ? '0' : ' ';
	return buf;
}

static char *write_number(char *buf, int flags, int width, int base, long long unsigned int value, int sign)
{
	int digits = 1;
	long long unsigned int exp = 1;
	if(base == 8)
		while(div8(value) >= exp)
		{
			exp = mul8(exp);
			digits++;
		}
	else if(base == 10)
		while(div10(value) >= exp)
		{
			exp = mul10(exp);
			digits++;
		}
	else if(base == 16)
		while(div16(value) >= exp)
		{
			exp = mul16(exp);
			digits++;
		}
	int digitwidth = width;
	if(flags & ALTERNATE)
	{
		if(base == 8)
			digitwidth--;
		else if(base == 16)
			digitwidth -= 2;
	}
	if((flags & (SIGN | BLANK)) || sign)
		digitwidth--;
	if(digitwidth < digits && width != -1)
		digits = digitwidth;
	int i;
	if(digits < 0)
	{
		for(i = 0; i < width; i++)
			*(buf++) = '?';
		return buf;
	}
	if(flags & ZERO_PAD)
	{
		if(sign)
			*(buf++) = '-';
		else if(flags & SIGN)
			*(buf++) = '+';
		else if(flags & BLANK)
			*(buf++) = ' ';
		buf = write_alternate(buf, flags, base);
	}
	buf = write_pad(buf, flags, digitwidth - digits, 1);
	if(!(flags & ZERO_PAD))
	{
		if(sign)
			*(buf++) = '-';
		else if(flags & SIGN)
			*(buf++) = '+';
		else if(flags & BLANK)
			*(buf++) = ' ';
		buf = write_alternate(buf, flags, base);
	}
	if(base == 8)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div8(exp);
			*(buf++) = digit + '0';
		}
	else if(base == 10)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div10(exp);
			*(buf++) = digit + '0';
		}
	else if(base == 16)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div16(exp);
			*(buf++) = digit >= 10 ? digit - 10 + (flags & UPPERCASE ? 'A' : 'a') : digit + '0';
		}
	buf = write_pad(buf, flags, digitwidth - digits, 0);
	return buf;
}

static char *write_signed(char *buf, int flags, int width, int type, int base, va_list arg)
{
	long long int value = 0;
	switch(type)
	{
	case DEFAULT: value = va_arg(arg, int); break;
	case HALF_HALF: value = va_arg(arg, int); break;
	case HALF: value = va_arg(arg, int); break;
	case LONG: value = va_arg(arg, long int); break;
	case LONG_LONG: value = va_arg(arg, long long int); break;
	case INTMAX_T: value = va_arg(arg, intmax_t); break;
	case SIZE_T: value = va_arg(arg, size_t); break;
	case PTRDIFF_T: value = va_arg(arg, ptrdiff_t); break;
	}
	int sign = value < 0;
	return write_number(buf, flags, width, base, sign ? -value : value, sign);
}

static char *write_unsigned(char *buf, int flags, int width, int type, int base, va_list arg)
{
	long long unsigned int value = 0;
	switch(type)
	{
	case DEFAULT: value = va_arg(arg, unsigned int); break;
	case HALF_HALF: value = va_arg(arg, unsigned int); break;
	case HALF: value = va_arg(arg, unsigned int); break;
	case LONG: value = va_arg(arg, long unsigned int); break;
	case LONG_LONG: value = va_arg(arg, long long unsigned int); break;
	case INTMAX_T: value = va_arg(arg, uintmax_t); break;
	case SIZE_T: value = va_arg(arg, size_t); break;
	case PTRDIFF_T: value = va_arg(arg, ptrdiff_t); break;
	}
	return write_number(buf, flags, width, base, value, 0);
}

size_t vsprintf(char *dest, char const *fmt, va_list arg)
{
	char *buf = dest;
	while(*fmt)
	{
		if(*fmt != '%')
			*(buf++) = *(fmt++);
		else
		{
			fmt++;
			int flags = 0;
			int type = DEFAULT;
			int width = -1;
			int precision = -1;
			while(*fmt == '#' || *fmt == '0' || *fmt == '-' || *fmt == ' ' || *fmt == '+')
			{
				if(*fmt == '#')
					flags |= ALTERNATE;
				else if(*fmt == '0')
					flags |= ZERO_PAD;
				else if(*fmt == '-')
					flags |= LEFT_ADJUST;
				else if(*fmt == ' ')
					flags |= BLANK;
				else if(*fmt == '+')
					flags |= SIGN;
				fmt++;
			}
			while(*fmt >= '0' && *fmt <= '9')
			{
				if(width == -1)
					width = 0;
				width = width * 10 + (*fmt - '0');
				fmt++;
			}
			if(*fmt == '*')
			{
				width = va_arg(arg, int);
				fmt++;
			}
			if(*fmt == '.')
			{
				fmt++;
				while(*fmt >= '0' && *fmt <= '9')
				{
					if(precision == -1)
						precision = 0;
					precision = precision * 10 + (*fmt - '0');
					fmt++;
				}
				if(*fmt == '*')
				{
					precision = va_arg(arg, int);
					fmt++;
				}
			}
			int done = 0;
			while(*fmt && !done)
			{
				switch(*fmt)
				{
				case 'h':
					type = type == HALF ? HALF_HALF : HALF;
					break;
				case 'l':
					type = type == LONG ? LONG_LONG : LONG;
					break;
				case 'j':
					type = INTMAX_T;
					break;
				case 'z':
					type = SIZE_T;
					break;
				case 't':
					type = PTRDIFF_T;
					break;
				case 'd': case 'i':
					buf = write_signed(buf, flags, width, type, 10, arg);
					done = 1;
					break;
				case 'o':
					buf = write_unsigned(buf, flags, width, type, 8, arg);
					done = 1;
					break;
				case 'u':
					buf = write_unsigned(buf, flags, width, type, 10, arg);
					done = 1;
					break;
				case 'x':
					buf = write_unsigned(buf, flags, width, type, 16, arg);
					done = 1;
					break;
				case 'X':
					flags |= UPPERCASE;
					buf = write_unsigned(buf, flags, width, type, 16, arg);
					done = 1;
					break;
				case 'p':
					flags |= ALTERNATE;
					buf = write_unsigned(buf, flags, width, type, 16, arg);
					done = 1;
					break;
				case 'c':
					buf = write_pad(buf, flags, width - 1, 1);
					*(buf++) = va_arg(arg, int);
					buf = write_pad(buf, flags, width - 1, 0);
					done = 1;
					break;
				case 's':
					{
						char const *str = va_arg(arg, char const *);
						int size = 0;
						while(str[size] && (precision == -1 || size < precision))
							size++;
						width = precision == -1 ? size : precision;
						buf = write_pad(buf, flags, width - size, 1);
						int i;
						for(i = 0; i < size; i++)
							*(buf++) = str[i];
						buf = write_pad(buf, flags, width - size, 0);
						done = 1;
						break;
					}
				case '%':
					*(buf++) = '%';
					done = 1;
					break;
				}
				fmt++;
			}
		}
	}
	*buf = 0;
	return buf - dest;
}

static void print_alternate(int flags, int base)
{
	if(flags & ALTERNATE)
	{
		if(base == 8)
			vga_put_char('0');
		else if(base == 16)
		{
			vga_put_char('0');
			vga_put_char('x');
		}
	}
}

static void print_pad(int flags, int size, int pre)
{
	int i;
	if(!(flags & LEFT_ADJUST) != !pre)
		for(i = 0; i < size; i++)
			vga_put_char(pre && (flags & ZERO_PAD) ? '0' : ' ');
}

static void print_number(int flags, int width, int base, long long unsigned int value, int sign)
{
	int digits = 1;
	long long unsigned int exp = 1;
	if(base == 8)
		while(div8(value) >= exp)
		{
			exp = mul8(exp);
			digits++;
		}
	else if(base == 10)
		while(div10(value) >= exp)
		{
			exp = mul10(exp);
			digits++;
		}
	else if(base == 16)
		while(div16(value) >= exp)
		{
			exp = mul16(exp);
			digits++;
		}
	int digitwidth = width;
	if(flags & ALTERNATE)
	{
		if(base == 8)
			digitwidth--;
		else if(base == 16)
			digitwidth -= 2;
	}
	if((flags & (SIGN | BLANK)) || sign)
		digitwidth--;
	if(digitwidth < digits && width != -1)
		digits = digitwidth;
	int i;
	if(digits < 0)
	{
		for(i = 0; i < width; i++)
			vga_put_char('?');
		return;
	}
	if(flags & ZERO_PAD)
	{
		if(sign)
			vga_put_char('-');
		else if(flags & SIGN)
			vga_put_char('+');
		else if(flags & BLANK)
			vga_put_char(' ');
		print_alternate(flags, base);
	}
	print_pad(flags, digitwidth - digits, 1);
	if(!(flags & ZERO_PAD))
	{
		if(sign)
			vga_put_char('-');
		else if(flags & SIGN)
			vga_put_char('+');
		else if(flags & BLANK)
			vga_put_char(' ');
		print_alternate(flags, base);
	}
	if(base == 8)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div8(exp);
			vga_put_char(digit + '0');
		}
	else if(base == 10)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div10(exp);
			vga_put_char(digit + '0');
		}
	else if(base == 16)
		for(i = 0; i < digits; i++)
		{
			int digit = 0;
			while(value >= exp)
			{
				digit++;
				value -= exp;
			}
			exp = div16(exp);
			vga_put_char(digit >= 10 ? digit - 10 + (flags & UPPERCASE ? 'A' : 'a') : digit + '0');
		}
	print_pad(flags, digitwidth - digits, 0);
}

static void print_signed(int flags, int width, int type, int base, va_list *arg)
{
	long long int value = 0;
	switch(type)
	{
	case DEFAULT: value = va_arg(*arg, int); break;
	case HALF_HALF: value = va_arg(*arg, int); break;
	case HALF: value = va_arg(*arg, int); break;
	case LONG: value = va_arg(*arg, long int); break;
	case LONG_LONG: value = va_arg(*arg, long long int); break;
	case INTMAX_T: value = va_arg(*arg, intmax_t); break;
	case SIZE_T: value = va_arg(*arg, size_t); break;
	case PTRDIFF_T: value = va_arg(*arg, ptrdiff_t); break;
	}
	int sign = value < 0;
	print_number(flags, width, base, sign ? -value : value, sign);
}
static void print_unsigned(int flags, int width, int type, int base, va_list *arg)
{
	long long unsigned int value = 0;
	switch(type)
	{
	case DEFAULT: value = va_arg(*arg, unsigned int); break;
	case HALF_HALF: value = va_arg(*arg, unsigned int); break;
	case HALF: value = va_arg(*arg, unsigned int); break;
	case LONG: value = va_arg(*arg, long unsigned int); break;
	case LONG_LONG: value = va_arg(*arg, long long unsigned int); break;
	case INTMAX_T: value = va_arg(*arg, uintmax_t); break;
	case SIZE_T: value = va_arg(*arg, size_t); break;
	case PTRDIFF_T: value = va_arg(*arg, ptrdiff_t); break;
	}
	print_number(flags, width, base, value, 0);
}

void vga_vprintf(char const *fmt, va_list arg)
{
	while(*fmt)
	{
		if(*fmt != '%')
			vga_put_char(*(fmt++));
		else
		{
			fmt++;
			int flags = 0;
			int type = DEFAULT;
			int width = -1;
			int precision = -1;
			while(*fmt == '#' || *fmt == '0' || *fmt == '-' || *fmt == ' ' || *fmt == '+')
			{
				if(*fmt == '#')
					flags |= ALTERNATE;
				else if(*fmt == '0')
					flags |= ZERO_PAD;
				else if(*fmt == '-')
					flags |= LEFT_ADJUST;
				else if(*fmt == ' ')
					flags |= BLANK;
				else if(*fmt == '+')
					flags |= SIGN;
				fmt++;
			}
			while(*fmt >= '0' && *fmt <= '9')
			{
				if(width == -1)
					width = 0;
				width = width * 10 + (*fmt - '0');
				fmt++;
			}
			if(*fmt == '*')
			{
				width = va_arg(arg, int);
				fmt++;
			}
			if(*fmt == '.')
			{
				fmt++;
				while(*fmt >= '0' && *fmt <= '9')
				{
					if(precision == -1)
						precision = 0;
					precision = precision * 10 + (*fmt - '0');
					fmt++;
				}
				if(*fmt == '*')
				{
					precision = va_arg(arg, int);
					fmt++;
				}
			}
			int done = 0;
			while(*fmt && !done)
			{
				switch(*fmt)
				{
				case 'h':
					type = type == HALF ? HALF_HALF : HALF;
					break;
				case 'l':
					type = type == LONG ? LONG_LONG : LONG;
					break;
				case 'j':
					type = INTMAX_T;
					break;
				case 'z':
					type = SIZE_T;
					break;
				case 't':
					type = PTRDIFF_T;
					break;
				case 'd': case 'i':
					print_signed(flags, width, type, 10, &arg);
					done = 1;
					break;
				case 'o':
					print_unsigned(flags, width, type, 8, &arg);
					done = 1;
					break;
				case 'u':
					print_unsigned(flags, width, type, 10, &arg);
					done = 1;
					break;
				case 'x':
					print_unsigned(flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'X':
					flags |= UPPERCASE;
					print_unsigned(flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'p':
					flags |= ALTERNATE;
					if(width != -1)
						width += 2;
					print_unsigned(flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'c':
					print_pad(flags, width - 1, 1);
					vga_put_char(va_arg(arg, int));
					print_pad(flags, width - 1, 0);
					done = 1;
					break;
				case 's':
					{
						char const *str = va_arg(arg, char const *);
						int size = 0;
						while(str[size] && (precision == -1 || size < precision))
							size++;
						width = precision == -1 ? size : precision;
						print_pad(flags, width - size, 1);
						int i;
						for(i = 0; i < size; i++)
							vga_put_char(str[i]);
						print_pad(flags, width - size, 0);
						done = 1;
						break;
					}
				case '%':
					vga_put_char('%');
					done = 1;
					break;
				}
				fmt++;
			}
		}
	}
	vga_sync_cursor();
}
