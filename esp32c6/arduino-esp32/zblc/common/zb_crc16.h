#ifndef ZB_CRC16_H
#define ZB_CRC16_H

#include <stdint.h>

void crc16_init(uint16_t *crc);
void crc16_update(uint16_t *crc, uint8_t *data, uint16_t length);
uint16_t crc16(uint8_t *data, uint16_t length);

#endif
