#ifndef LIBC_SETJMP_H_
#define LIBC_SETJMP_H_

struct jmp_buf_
{
	void *ebx, *esi, *edi, *ebp, *esp, *eip;
};
typedef struct jmp_buf_ jmp_buf[1];

extern void longjmp(jmp_buf, int);
extern int setjmp(jmp_buf);

#endif
