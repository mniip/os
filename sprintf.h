#ifndef SPRINTF_H
#define SPRINTF_H

#include <stddef.h>
#include <stdarg.h>

void *vstreamf(void *(*)(void *, char), void *, char const *, va_list);
extern int sprintf(char *, char const *, ...);
extern int vsprintf(char *, char const *, va_list);

#endif
