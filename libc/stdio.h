#ifndef LIBC_STDIO_H_
#define LIBC_STDIO_H_

#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#define BUFSIZ 4096
#define L_tmpnam 1

#define EOF (-1)

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IONBF 0
#define _IOFBF 0
#define _IOLBF 0

typedef struct
{
	int driver;
	int error;
	void *data;
	uint32_t size;
	uint32_t cursor;
	char *filename;
}
FILE;

static const FILE *stdin = NULL;
static const FILE *stdout = NULL;
static const FILE *stderr = NULL;

extern int feof(FILE *);
extern int ferror(FILE *);
extern void clearerr(FILE *);

extern FILE *fopen(char const *, char const *);
extern FILE *freopen(char const *, char const *, FILE *);
extern size_t fread(void *, size_t, size_t, FILE *);
extern int fgetc(FILE *);
extern size_t fwrite(void const *, size_t, size_t, FILE *);
extern int fprintf(FILE *, char const *, ...);
extern int fseek(FILE *, long, int);
extern long ftell(FILE *);
extern int fflush(FILE *);
extern int fclose(FILE *);
extern int remove(char const *);
extern int rename(char const *, char const *);

#define getc fgetc

static inline FILE *tmpfile() { errno = ENOTSUP; return NULL; }
static inline char *tmpnam(char *s) { errno = ENOTSUP; return NULL; }
static inline int ungetc(int c, FILE *stream) { errno = ENOTSUP; return EOF; }
static inline int setvbuf(FILE *stream, char *buf, int mode, size_t size) {}

extern int snprintf(char *, size_t, char const *, ...);
extern int fprintf(FILE *, char const *, ...);

#endif
