#ifndef LIBC_STDLIB_H_
#define LIBC_STDLIB_H_

#include <stddef.h>
#include <errno.h>

#define RAND_MAX 65535

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 0

static inline int rand() { return 4; }
static inline int system(const char *command) { errno = ENOTSUP; return -1; }
static inline void srand(unsigned int seed) {}
static inline void exit(int status) {}
static inline char *getenv(const char *name) { return NULL; }

#endif
