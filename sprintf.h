#ifndef SPRINTF_H
#define SPRINTF_H

#include <stddef.h>
#include <stdarg.h>

extern size_t sprintf(char *, char const *, ...);
extern size_t vsprintf(char *, char const *, va_list);

#endif
