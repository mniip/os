#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>

int int13_02(uint8_t drive, uint8_t head, uint16_t cylsect, uint8_t sectors, void *dest, uint8_t *retcode, uint8_t *act_sectors);
int int13_03(uint8_t drive, uint8_t head, uint16_t cylsect, uint8_t sectors, void const *dest, uint8_t *retcode, uint8_t *act_sectors);
int int13_08(uint8_t drive, uint8_t *retcode, uint8_t *head, uint16_t *cylsect);

#endif
