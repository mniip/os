#ifndef DISK_H
#define DISK_H

extern uint8_t drive;

typedef struct
{
	int sectors, heads, cylinders;
}
disk_chs;

int disk_get_chs(uint8_t, disk_chs *);
int disk_read(uint8_t, disk_chs const *, int, void *);
int disk_write(uint8_t, disk_chs const *, int, void const *);

#endif
