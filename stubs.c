#include <stdint.h>
#include <locale.h>
#include <setjmp.h>

struct lconv *localeconv()
{
	static struct lconv s;
	s.decimal_point = ".";
	return &s;
}

uint32_t time(uint32_t *t)
{
	if(t)
		*t = 0;
	return 0;
}

void abort()
{
}

double fmod(double x, double y)
{
	if(x != x || y != y)
		return x;
	if(y == 0.0)
		return 0.0/0.0;
	return x - (int)(x / y) * y;
}

double frexp(double x, int *exp)
{
	if(exp)
		*exp = 0;
	return x;
}

asm(
".global pow\n"
"pow:\n"
"\tpush %ebp\n"
"\tmov %esp, %ebp\n"
"\tsub $4, %esp\n"
"\tfldl 16(%ebp)\n"
"\tfldl 8(%ebp)\n"
"\tfyl2x\n"
"\tfist (%esp)\n"
"\tfild (%esp)\n"
"\tfsubp\n"
"\tf2xm1\n"
"\tfld1\n"
"\tfaddp\n"
"\tfild (%esp)\n"
"\tfxch\n"
"\tfscale\n"
"\tleave\n"
"\tret\n"
);

double floor(double x)
{
	return (int)x;
}
