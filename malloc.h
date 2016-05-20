#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>

extern void init_alloc();
extern void print_free_list();
extern void *malloc(uint32_t);
extern void free(void *);

#endif
