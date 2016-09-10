#ifndef LIBC_STDIO_H_
#define LIBC_STDIO_H_

#include <stddef.h>
#include <errno.h>

#define BUFSIZ 4096
#define L_tmpnam 1

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 0
#define SEEK_END 0

#define _IONBF 0
#define _IOFBF 0
#define _IOLBF 0

typedef struct FILE_ FILE;

static FILE *stdin = NULL;
static FILE *stdout = NULL;
static FILE *stderr = NULL;

static inline int feof(FILE *stream) { return 0; }
static inline size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) { errno = ENOTSUP; return EOF; }
static inline size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) { errno = ENOTSUP; return EOF; }
static inline FILE *fopen(const char *path, const char *mode) { errno = ENOTSUP; return NULL; }
static inline FILE *freopen(const char *path, const char *mode, FILE *stream) { errno = ENOTSUP; return NULL; }
static inline FILE *tmpfile() { errno = ENOTSUP; return NULL; }
static inline char *tmpnam(char *s) { errno = ENOTSUP; return NULL; }
static inline int getc(FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int fflush(FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int fclose(FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int fprintf(FILE *stream, const char *format, ...) { errno = ENOTSUP; return EOF; }
static inline int ungetc(int c, FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int ferror(FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int fseek(FILE *stream, long offset, int whence) { errno = ENOTSUP; return EOF; }
static inline int setvbuf(FILE *stream, char *buf, int mode, size_t size) { errno = ENOTSUP; return EOF; }
static inline int remove(const char *pathname) { errno = ENOTSUP; return EOF; }
static inline int rename(const char *old, const char *new) { errno = ENOTSUP; return EOF; }
static inline long ftell(FILE *stream) { errno = ENOTSUP; return EOF; }
static inline void clearerr(FILE *stream) {}

extern int snprintf(char *str, size_t size, const char *format, ...);

#endif
