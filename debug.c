#include <stdint.h>
#include "vga_io.h"

typedef struct
{
	uint16_t offset_1;
	uint16_t selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t offset_2;
}
IDT_entry;

extern void handler_UNK();
extern void handler_DE();
extern void handler_DB();
extern void handler_NMI();
extern void handler_BP();
extern void handler_OF();
extern void handler_BR();
extern void handler_UD();
extern void handler_NM();
extern void handler_DF();
extern void handler_TS();
extern void handler_NP();
extern void handler_SS();
extern void handler_GP();
extern void handler_PF();
extern void handler_MF();
extern void handler_AC();
extern void handler_MC();
extern void handler_XM();
extern void handler_VE();
extern void handler_SX();
void (*(handlers[32]))() =
{
	handler_DE,
	handler_DB,
	handler_NMI,
	handler_BP,
	handler_OF,
	handler_BR,
	handler_UD,
	handler_NM,
	handler_DF,
	handler_UNK,
	handler_TS,
	handler_NP,
	handler_SS,
	handler_GP,
	handler_PF,
	handler_UNK,
	handler_MF,
	handler_AC,
	handler_MC,
	handler_XM,
	handler_VE,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_UNK,
	handler_SX,
};
IDT_entry IDT[32];

void init_handlers()
{
	int i;
	for(i = 0; i < 32; i++)
	{
		IDT[i].offset_1 = (uint32_t)handlers[i] & 0xFFFF;
		IDT[i].offset_2 = (uint32_t)handlers[i] >> 16;
		IDT[i].selector = 0x10;
		IDT[i].reserved = 0;
		IDT[i].flags = 0x8E;
	}
	char idtr[6];
	*(uint16_t *)idtr = sizeof(IDT) - 1;
	*(void **)(idtr + 2) = &IDT;
	__asm__ __volatile__("lidt (%0)" : : "r"(&idtr));
	vga_printf("Loaded IDT\n");
}

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
