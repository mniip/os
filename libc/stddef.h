#ifndef LIBC_STDDEF_H_
#define LIBC_STDDEF_H_

#include <stdint.h>

#define NULL ((void *)0)
#define offsetof(s, m) ((void *)&(((s *)NULL)->m) - NULL)

typedef uint32_t size_t;
typedef int32_t ptrdiff_t;

#endif
