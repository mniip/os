#include <lua.h>

void print_backtrace()
{
	vga_printf("[begin stack trace]\n");
	int i;
	void *frame;
	asm("mov %%ebp, %0" : "=r"(frame));
	for(i = 0; ((void **)frame)[0]; i++)
	{
		vga_printf("%3d: ebp=%x -> eip=%x\n", i, frame, ((void **)frame)[1]);
		frame = ((void **)frame)[0];
	}
	vga_printf("[end stack trace]\n");
}
