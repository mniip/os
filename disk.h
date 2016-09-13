#ifndef DISK_H
#define DISK_H

#include <stdint.h>

extern uint8_t drive;

typedef struct
{
	int sectors, heads, cylinders;
}
disk_chs;

int disk_get_chs(uint8_t, disk_chs *);
int disk_read(uint8_t, disk_chs const *, int, void *);
int disk_write(uint8_t, disk_chs const *, int, void const *);

typedef struct
{
	uint32_t size;
	char name[17];
	uint8_t type;
	uint16_t sector;
}
dir_entry;

enum
{
	TYPE_FILE = 1,
	TYPE_DIR = 2,
};

void *file_read_whole(char const *, uint32_t *);
void file_write_whole(char const *, void const *, uint32_t);
dir_entry *file_list(char const *);
void file_mkdir(char const *);
void file_remove(char const *);

#endif
