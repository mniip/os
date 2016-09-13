#include "disk.h"
#include "malloc.h"
#include "string.h"
#include "bios.h"
#include "vga_io.h"

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

extern char _end;
extern char _start;
static int get_fat_sector()
{
	return 1 + 1 + (((uint32_t)&_end - (uint32_t)&_start) >> 9);
}

uint16_t *fat_cache(uint8_t disk, disk_chs const *chs)
{
	char data[0x200];
	if(disk_read(disk, chs, get_fat_sector(), data))
		return NULL;
	uint16_t *fat = malloc(0x200);
	memcpy(fat, data, 0x200);
	return fat;
}

void fat_sync(uint8_t disk, disk_chs const *chs, uint16_t *fat)
{
	char data[0x200];
	memcpy(data, fat, 0x200);
	disk_write(disk, chs, get_fat_sector(), data);
}

uint16_t fat_find_free_sector(uint16_t *fat)
{
	uint16_t sector;
	for(sector = 1; sector < 0x100; sector++)
		if(!(fat[sector] & 0x8000))
			return sector;
	return 0;
}

void fat_free_chain(uint16_t *fat, uint16_t sector)
{
	while(sector && (fat[sector] & 0x8000))
		sector = (fat[sector] &= 0x7fff);
}

void *file_cached_read_whole(uint8_t disk, disk_chs const *chs, uint16_t *fat, uint16_t sector, uint32_t *size)
{
	char data[0x200];
	void *file = malloc(1);
	*size = 0;
	while(sector && (fat[sector] & 0x8000))
	{
		file = realloc(file, *size + 0x200);
		disk_read(disk, chs, get_fat_sector() + sector, data);
		memcpy(file + *size, data, 0x200);
		*size += 0x200;
		sector = fat[sector] & 0x7fff;
	}
	return file;
}

void file_cached_write_whole(uint8_t disk, disk_chs const *chs, uint16_t *fat, uint16_t sector, void const *file, uint32_t size)
{
	char data[0x200];
	fat_free_chain(fat, sector);
	while(size)
	{
		memcpy(data, file, 0x200);
		disk_write(disk, chs, get_fat_sector() + sector, data);
		fat[sector] = 0x8000;
		uint16_t next_sector = fat_find_free_sector(fat);
		if(size < 0x200)
			break;
		size -= 0x200;
		file += 0x200;
		fat[sector] = 0x8000 | next_sector;
		sector = next_sector;
	}
}

uint16_t fat_lookup_dir(uint8_t disk, disk_chs const *chs, uint16_t *fat, char const *path, char const *end)
{
	uint16_t sector = 1;
	char const *slash;
	while((slash = strchr(path, '/')) && (!end || slash <= end))
	{
		if(slash != path)
		{
			uint32_t size;
			dir_entry *dir = file_cached_read_whole(disk, chs, fat, sector, &size);
			if(!dir)
				return 0;
			uint32_t length = slash - path;
			if(length > sizeof dir[0].name)
				length = sizeof dir[0].name;
			int i;
			for(i = 0; i * sizeof(dir_entry) < size && dir[i].type; i++)
				if(!strncmp(dir[i].name, path, length))
				{
					if(dir[i].type != TYPE_DIR)
					{
						free(dir);
						return 0;
					}
					sector = dir[i].sector;
					free(dir);
					break;
				}
			if(!(i * sizeof(dir_entry) < size && dir[i].type))
			{
				free(dir);
				return 0;
			}
		}
		path = slash + 1;
	}
	return sector;
}

void *file_read_whole(char const *path, uint32_t *sz)
{
	char const *slash = strrchr(path, '/');
	char const *basename = slash ? slash + 1 : path;
	if(!slash)
		slash = path;
	disk_chs chs;
	int result = disk_get_chs(drive, &chs);
	if(result)
		return NULL;
	uint16_t *fat = fat_cache(drive, &chs);
	if(!fat)
		return NULL;
	uint16_t sector = fat_lookup_dir(drive, &chs, fat, path, slash);
	if(!sector)
	{
		free(fat);
		return NULL;
	}
	uint32_t size;
	dir_entry *dir = file_cached_read_whole(drive, &chs, fat, sector, &size);
	if(!dir)
	{
		free(fat);
		return NULL;
	}
	uint32_t length = strlen(basename) + 1;
	if(length > sizeof dir[0].name)
		length = sizeof dir[0].name;
	int i;
	uint16_t filesector;
	for(i = 0; i * sizeof(dir_entry) < size && dir[i].type; i++)
	{
		if(!strncmp(dir[i].name, basename, length))
		{
			if(dir[i].type != TYPE_FILE)
			{
				free(dir);
				free(fat);
				return NULL;
			}
			filesector = dir[i].sector;
			break;
		}
	}
	if(!(i * sizeof(dir_entry) < size && dir[i].type))
	{
		free(dir);
		free(fat);
		return NULL;
	}
	*sz = dir[i].size;
	free(dir);
	void *file = file_cached_read_whole(drive, &chs, fat, filesector, &size);
	free(fat);
	return file;
}

void file_write_whole(char const *path, void const *file, uint32_t sz)
{
	char const *slash = strrchr(path, '/');
	char const *basename = slash ? slash + 1 : path;
	if(!slash)
		slash = path;
	disk_chs chs;
	int result = disk_get_chs(drive, &chs);
	if(result)
		return;
	uint16_t *fat = fat_cache(drive, &chs);
	if(!fat)
		return;
	uint16_t sector = fat_lookup_dir(drive, &chs, fat, path, slash);
	if(!sector)
	{
		free(fat);
		return;
	}
	uint32_t size;
	dir_entry *dir = file_cached_read_whole(drive, &chs, fat, sector, &size);
	if(!dir)
	{
		free(fat);
		return;
	}
	uint32_t length = strlen(basename) + 1;
	if(length > sizeof dir[0].name)
		length = sizeof dir[0].name;
	int i;
	uint16_t filesector;
	for(i = 0; i * sizeof(dir_entry) < size && dir[i].type; i++)
		if(!strncmp(dir[i].name, basename, length))
		{
			if(dir[i].type != TYPE_FILE)
			{
				free(dir);
				free(fat);
				return;
			}
			filesector = dir[i].sector;
			dir[i].size = sz;
			file_cached_write_whole(drive, &chs, fat, sector, dir, size);
			break;
		}
	if(!(i * sizeof(dir_entry) < size && dir[i].type))
	{
		dir = realloc(dir, sizeof(dir_entry) * (i + 2));
		dir[i].size = sz;
		memcpy(dir[i].name, basename, length);
		dir[i].type = TYPE_FILE;
		fat[sector] |= 0x8000;
		filesector = fat_find_free_sector(fat);
		dir[i].sector = filesector;
		dir[i + 1].type = 0;
		file_cached_write_whole(drive, &chs, fat, sector, dir, sizeof(dir_entry) * (i + 2));
	}
	free(dir);
	file_cached_write_whole(drive, &chs, fat, filesector, file, sz);
	fat_sync(drive, &chs, fat);
	free(fat);
}

dir_entry *file_list(char const *path)
{
	disk_chs chs;
	int result = disk_get_chs(drive, &chs);
	if(result)
		return NULL;
	uint16_t *fat = fat_cache(drive, &chs);
	if(!fat)
		return NULL;
	uint16_t sector = fat_lookup_dir(drive, &chs, fat, path, NULL);
	if(!sector)
	{
		free(fat);
		return NULL;
	}
	uint32_t size;
	dir_entry *dir = file_cached_read_whole(drive, &chs, fat, sector, &size);
	free(fat);
	int last = size / sizeof(dir_entry);
	dir = realloc(dir, sizeof(dir_entry) * (1 + last));
	dir[last].type = 0;
	return dir;
}

void file_mkdir(char const *path)
{
	char const *slash = strrchr(path, '/');
	char const *basename = slash ? slash + 1 : path;
	if(!slash)
		slash = path;
	disk_chs chs;
	int result = disk_get_chs(drive, &chs);
	if(result)
		return;
	uint16_t *fat = fat_cache(drive, &chs);
	if(!fat)
		return;
	uint16_t sector = fat_lookup_dir(drive, &chs, fat, path, slash);
	if(!sector)
	{
		free(fat);
		return;
	}
	uint32_t size;
	dir_entry *dir = file_cached_read_whole(drive, &chs, fat, sector, &size);
	if(!dir)
	{
		free(fat);
		return;
	}
	uint32_t length = strlen(basename) + 1;
	if(length > sizeof dir[0].name)
		length = sizeof dir[0].name;
	int i;
	uint16_t filesector;
	for(i = 0; i * sizeof(dir_entry) < size && dir[i].type; i++)
		if(!strncmp(dir[i].name, basename, length))
		{
			free(dir);
			free(fat);
			return;
		}
	dir = realloc(dir, sizeof(dir_entry) * (i + 2));
	memcpy(dir[i].name, basename, length);
	dir[i].type = TYPE_DIR;
	fat[sector] |= 0x8000;
	filesector = fat_find_free_sector(fat);
	dir[i].sector = filesector;
	dir[i + 1].type = 0;
	file_cached_write_whole(drive, &chs, fat, sector, dir, sizeof(dir_entry) * (i + 2));
	free(dir);
	dir_entry newdir;
	newdir.type = 0;
	file_cached_write_whole(drive, &chs, fat, filesector, &newdir, sizeof newdir);
	fat_sync(drive, &chs, fat);
	free(fat);
}

void file_remove(char const *path)
{
	char const *slash = strrchr(path, '/');
	char const *basename = slash ? slash + 1 : path;
	if(!slash)
		slash = path;
	disk_chs chs;
	int result = disk_get_chs(drive, &chs);
	if(result)
		return;
	uint16_t *fat = fat_cache(drive, &chs);
	if(!fat)
		return;
	uint16_t sector = fat_lookup_dir(drive, &chs, fat, path, slash);
	if(!sector)
	{
		free(fat);
		return;
	}
	uint32_t size;
	dir_entry *dir = file_cached_read_whole(drive, &chs, fat, sector, &size);
	if(!dir)
	{
		free(fat);
		return;
	}
	uint32_t length = strlen(basename) + 1;
	if(length > sizeof dir[0].name)
		length = sizeof dir[0].name;
	int i;
	for(i = 0; i * sizeof(dir_entry) < size && dir[i].type; i++)
		if(!strncmp(dir[i].name, basename, length))
		{
			if(dir[i].type == TYPE_DIR)
			{
				uint32_t subsize;
				dir_entry *subdir = file_cached_read_whole(drive, &chs, fat, dir[i].sector, &subsize);
				if(subdir && subsize && subdir[0].type)
				{
					free(subdir);
					free(dir);
					free(fat);
					return;
				}
				free(subdir);
			}
			fat_free_chain(fat, dir[i].sector);
			int j;
			for(j = i + 1; j * sizeof(dir_entry) < size && dir[j].type; j++)
				dir[j - 1] = dir[j];
			dir[j - 1].type = 0;
			file_cached_write_whole(drive, &chs, fat, sector, dir, sizeof(dir_entry) * (j + 1));
			free(dir);
			free(fat);
			return;
		}
	free(dir);
	free(fat);
}
