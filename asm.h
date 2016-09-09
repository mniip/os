#ifndef ASM_H
#define ASM_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t data)
{
	__asm__ __volatile__("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t data)
{
	__asm__ __volatile__("outw %0, %1" : : "a"(data), "Nd"(port));
}

static inline void outl(uint16_t port, uint32_t data)
{
	__asm__ __volatile__("outl %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void pause()
{
	__asm__ __volatile__("pause");
}

#endif
