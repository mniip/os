#include <stdint.h>
#include "vga_io.h"
#include "malloc.h"

void main()
{
	init_alloc();

	print_free_list();
	void *foo = malloc(0x1000);
	print_free_list();
	void *bar = malloc(0x2000);
	print_free_list();
	free(foo);
	print_free_list();
	void *baz = malloc(0x3000);
	print_free_list();
	free(bar);
	print_free_list();
	free(baz);
	print_free_list();
}
