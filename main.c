#include <stdint.h>
#include <limits.h>
#include "vga_io.h"
#include "malloc.h"
#include "sprintf.h"

void main()
{
	init_alloc();

	vga_printf("&main = %08p\n", &main);

	int i;
	for(i = 10; i >= 0; i--)
		vga_printf("'%#0*x' %.*s '%*d' %.*s '%.*s' %.*s '%d'\n", i, 0xdead, 10 - i, "", i, -123, 10 - i, "", i, "moo", 10 - i, "", i << i);

	vga_printf("%d %d %lld %lld %u %llu\n", INT_MIN, INT_MAX, LLONG_MIN, LLONG_MAX, UINT_MAX, ULLONG_MAX);

	vga_set_color(0xC, 0x0);
	vga_printf("HALT");
}
