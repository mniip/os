#include "libc/stdio.h"
#include "libc/errno.h"
#include <string.h>

#include "malloc.h"
#include "vga_io.h"
#include "keyboard.h"
#include "disk.h"

enum
{
	DRIVER_FILE,
};

enum
{
	E_NOERROR,
	E_ERROR,
	E_EOF,
};

FILE *fopen(char const *path, char const *mode)
{
	uint32_t size;
	void *data = file_read_whole(path, &size);
	if(!data)
		if(strchr(mode, 'w') || strchr(mode, 'a'))
			size = 0;
		else
			return NULL;
	FILE *r = malloc(sizeof(FILE));
	r->driver = DRIVER_FILE;
	r->error = E_NOERROR;
	r->data = data;
	r->size = size;
	r->cursor = strchr(mode, 'a') ? size : 0;
	r->filename = malloc(strlen(path) + 1);
	strcpy(r->filename, path);
	return r;
}

FILE *freopen(char const *path, char const *mode, FILE *file)
{
	uint32_t size;
	void *data = file_read_whole(path, &size);
	if(!data)
		if(strchr(mode, 'w') || strchr(mode, 'a'))
			size = 0;
		else
			return NULL;
	if(file)
	{
		if(file->driver == DRIVER_FILE)
		{
			file_write_whole(file->filename, file->data, file->size);
			free(file->filename);
		}
	}
	else
	{
		free(data);
		errno = EBADF;
		return NULL;
	}
	file->driver = DRIVER_FILE;
	file->error = E_NOERROR;
	file->data = data;
	file->size = size;
	file->cursor = strchr(mode, 'a') ? size : 0;
	file->filename = malloc(strlen(path) + 1);
	strcpy(file->filename, path);
	return file;
}

int ferror(FILE *file)
{
	if(file)
		return file->error == E_ERROR;
	errno = EBADF;
	return 0;
}

int feof(FILE *file)
{
	if(file)
		return file->error == E_EOF;
	errno = EBADF;
	return 0;
}

void clearerr(FILE *file)
{
	if(file)
		file->error = E_NOERROR;
}

int fclose(FILE *file)
{
	if(file)
	{
		if(file->driver == DRIVER_FILE)
		{
			file_write_whole(file->filename, file->data, file->size);
			free(file->data);
			free(file->filename);
		}
		free(file);
		return 0;
	}
	errno = EBADF;
	return -1;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if(stream)
	{
		if(stream->driver == DRIVER_FILE)
		{
			size_t i;
			for(i = 0; i < nmemb; i++)
				if(stream->cursor + size <= stream->size)
				{
					memcpy(ptr, stream->data + stream->cursor, size);
					ptr += size;
					stream->cursor += size;
				}
				else
				{
					stream->error = E_EOF;
					break;
				}
			return i;
		}
		errno = ENOTSUP;
		return 0;
	}
	errno = EBADF;
	return 0;
}

int fgetc(FILE *file)
{
	if(file)
	{
		if(file->driver == DRIVER_FILE)
		{
			if(file->cursor < file->size)
			{
				unsigned char c = *(unsigned char *)(file->data + file->cursor);
				file->cursor++;
				return c;
			}
			else
			{
				file->error = E_EOF;
				return EOF;
			}
		}
		errno = ENOTSUP;
		return 0;
	}
	errno = EBADF;
	return 0;
}

size_t fwrite(void const *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if(stream)
	{
		if(stream->driver == DRIVER_FILE)
		{
			if(stream->cursor + size * nmemb > stream->size)
			{
				stream->data = realloc(stream->data, stream->cursor + size * nmemb);
				stream->size = stream->cursor + size * nmemb;
			}
			size_t i;
			for(i = 0; i < nmemb; i++)
			{
				memcpy(stream->data + stream->cursor, ptr, size);
				ptr += size;
				stream->cursor += size;
			}
			return i;
		}
		errno = ENOTSUP;
		return 0;
	}
	errno = EBADF;
	return 0;
}

int fflush(FILE *file)
{
	if(file)
	{
		if(file->driver == DRIVER_FILE)
			file_write_whole(file->filename, file->data, file->size);
		return 0;
	}
	errno = EBADF;
	return -1;
}

long ftell(FILE *file)
{
	if(file)
	{
		if(file->driver == DRIVER_FILE)
			return file->cursor;
		errno = ENOTSUP;
		return -1;
	}
	errno = EBADF;
	return -1;
}

int fseek(FILE *file, long offset, int whence)
{
	if(file)
	{
		if(file->driver == DRIVER_FILE)
		{
			long target = offset;
			if(whence == SEEK_CUR)
				target = file->cursor + offset;
			else if(whence == SEEK_END)
				target = file->size + offset;
			if(target < 0 || target > file->size)
			{
				errno = EINVAL;
				return -1;
			}
			file->cursor = target;
			return 0;
		}
		errno = ENOTSUP;
		return -1;
	}
	errno = EBADF;
	return -1;
}

typedef struct
{
	FILE *file;
	int pos;
	char buf[128];
}
fprintf_buf;

static void *fprintf_buf_writer(void *ud, char c)
{
	fprintf_buf *buf = (fprintf_buf *)ud;
	buf->buf[buf->pos % sizeof buf->buf] = c;
	buf->pos++;
	if(!(buf->pos % sizeof buf->buf))
		fwrite(buf->buf, sizeof buf->buf, 1, buf->file);
	return ud;
}

int vfprintf(FILE *file, char const *fmt, va_list arg)
{
	fprintf_buf buf;
	buf.file = file;
	buf.pos = 0;
	vstreamf(fprintf_buf_writer, (void *)&buf, fmt, arg);
	return buf.pos;
}

int fprintf(FILE *file, char const *fmt, ...)
{
	va_list arg;
	va_start(arg, fmt);
	int ret = vfprintf(file, fmt, arg);
	va_end(arg);
	return ret;
}
