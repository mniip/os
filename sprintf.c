#include <stdint.h>
#include <stddef.h>
#include "vga_io.h"
#include "sprintf.h"

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

static void *stream_alternate(void *(*writer)(void *, char), void *ud, int flags, int base)
{
	if(flags & ALTERNATE)
	{
		if(base == 8)
			ud = writer(ud, '0');
		else if(base == 16)
			ud = writer(writer(ud, '0'), 'x');
	}
	return ud;
}

static void *stream_pad(void *(*writer)(void *, char), void *ud, int flags, int size, int pre)
{
	int i;
	if(!(flags & LEFT_ADJUST) != !pre)
		for(i = 0; i < size; i++)
			ud = writer(ud, pre && (flags & ZERO_PAD) ? '0' : ' ');
	return ud;
}

static void *stream_number(void *(*writer)(void *, char), void *ud, int flags, int width, int base, long long unsigned int value, int sign)
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
			ud = writer(ud, '?');
		return ud;
	}
	if(flags & ZERO_PAD)
	{
		if(sign)
			ud = writer(ud, '-');
		else if(flags & SIGN)
			ud = writer(ud, '+');
		else if(flags & BLANK)
			ud = writer(ud, ' ');
		ud = stream_alternate(writer, ud, flags, base);
	}
	ud = stream_pad(writer, ud, flags, digitwidth - digits, 1);
	if(!(flags & ZERO_PAD))
	{
		if(sign)
			ud = writer(ud, '-');
		else if(flags & SIGN)
			ud = writer(ud, '+');
		else if(flags & BLANK)
			ud = writer(ud, ' ');
		ud = stream_alternate(writer, ud, flags, base);
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
			ud = writer(ud, digit + '0');
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
			ud = writer(ud, digit + '0');
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
			ud = writer(ud, digit >= 10 ? digit - 10 + (flags & UPPERCASE ? 'A' : 'a') : digit + '0');
		}
	ud = stream_pad(writer, ud, flags, digitwidth - digits, 0);
	return ud;
}

static void *stream_signed(void *(*writer)(void *, char), void *ud, int flags, int width, int type, int base, va_list *arg)
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
	return stream_number(writer, ud, flags, width, base, sign ? -value : value, sign);
}

static void *stream_unsigned(void *(*writer)(void *, char), void *ud, int flags, int width, int type, int base, va_list *arg)
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
	return stream_number(writer, ud, flags, width, base, value, 0);
}

void *vstreamf(void *(*writer)(void *, char), void *ud, char const *fmt, va_list arg)
{
	while(*fmt)
	{
		if(*fmt != '%')
			ud = writer(ud, *(fmt++));
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
					ud = stream_signed(writer, ud, flags, width, type, 10, &arg);
					done = 1;
					break;
				case 'o':
					ud = stream_unsigned(writer, ud, flags, width, type, 8, &arg);
					done = 1;
					break;
				case 'u':
					ud = stream_unsigned(writer, ud, flags, width, type, 10, &arg);
					done = 1;
					break;
				case 'x':
					ud = stream_unsigned(writer, ud, flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'X':
					flags |= UPPERCASE;
					ud = stream_unsigned(writer, ud, flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'p':
					flags |= ALTERNATE;
					ud = stream_unsigned(writer, ud, flags, width, type, 16, &arg);
					done = 1;
					break;
				case 'c':
					ud = stream_pad(writer, ud, flags, width - 1, 1);
					ud = writer(ud, va_arg(arg, int));
					ud = stream_pad(writer, ud, flags, width - 1, 0);
					done = 1;
					break;
				case 's':
					{
						char const *str = va_arg(arg, char const *);
						int size = 0;
						while(str[size] && (precision == -1 || size < precision))
							size++;
						width = precision == -1 ? size : precision;
						ud = stream_pad(writer, ud, flags, width - size, 1);
						int i;
						for(i = 0; i < size; i++)
							ud = writer(ud, str[i]);
						ud = stream_pad(writer, ud, flags, width - size, 0);
						done = 1;
						break;
					}
				case '%':
					ud = writer(ud, '%');
					done = 1;
					break;
				}
				fmt++;
			}
		}
	}
	return ud;
}

static void *string_writer(void *ud, char c)
{
	char *str = ud;
	*(str++) = c;
	return (void *)str;
}

int vsprintf(char *buf, char const *fmt, va_list arg)
{
	char *ret = vstreamf(string_writer, (void *)buf, fmt, arg);
	*ret = 0;
	return ret - buf;
}

int sprintf(char *buf, char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	int ret = vsprintf(buf, fmt, arg);
	va_end(arg);
	return ret;
}

static void *vga_writer(void *ud, char c)
{
	(*(int *)ud)++;
	vga_put_char(c);
	return ud;
}

int vga_vprintf(char const *fmt, va_list arg)
{
	int count = 0;
	vstreamf(vga_writer, (void *)&count, fmt, arg);
	vga_sync_cursor();
	return count;
}

int vga_printf(char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	int ret = vga_vprintf(fmt, arg);
	va_end(arg);
	return ret;
}
