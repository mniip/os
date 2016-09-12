#include "bios.h"
#include "vga_io.h"

typedef struct
{
	int sectors, heads, cylinders;
}
disk_chs;

int disk_get_chs(uint8_t disk, disk_chs *chs)
{
	uint8_t retcode;
	uint8_t heads;
	uint16_t cylsect;
	int success = int13_08(disk, &retcode, &heads, &cylsect);
	if(!success)
		return retcode;
	chs->sectors = cylsect & 0x3F;
	chs->heads = heads + 1;
	chs->cylinders = (cylsect >> 8 | (cylsect & 0xC0) << 2) + 1;
	return 0;
}

int disk_read(uint8_t disk, disk_chs const *chs, int index, void *dest)
{
	int sector = index % chs->sectors + 1;
	int head = index / chs->sectors % chs->heads;
	int cylinder = index / chs->sectors / chs->heads;
	uint16_t cylsect = sector | (cylinder & 0xFF) << 8 | (cylinder & 0x300) >> 2;
	uint8_t retcode;
	uint8_t act;
	int success = int13_02(disk, head, cylsect, 1, dest, &retcode, &act);
	if(!success)
		return retcode;
	if(!act)
		return -1;
	return 0;
}

int disk_write(uint8_t disk, disk_chs const *chs, int index, void const *dest)
{
	int sector = index % chs->sectors + 1;
	int head = index / chs->sectors % chs->heads;
	int cylinder = index / chs->sectors / chs->heads;
	uint16_t cylsect = sector | (cylinder & 0xFF) << 8 | (cylinder & 0x300) >> 2;
	uint8_t retcode;
	uint8_t act;
	int success = int13_03(disk, head, cylsect, 1, dest, &retcode, &act);
	if(!success)
		return retcode;
	if(!act)
		return -1;
	return 0;
}
