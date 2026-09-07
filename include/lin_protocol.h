#ifndef LIN_PROTOCOL_H
#define LIN_PROTOCOL_H

#include <stdint.h>

#define LIN_ID_SWITCH_STATE   0x10
#define LIN_ID_SLAVE_STATUS   0x11

#define LIN_SYNC_BYTE         0x55
#define LIN_MAX_DATA_LEN      2

uint8_t LIN_ComputePID(uint8_t id);
uint8_t LIN_ComputeChecksum(uint8_t pid, const uint8_t *data, uint8_t len);
uint8_t LIN_ValidateChecksum(uint8_t pid, const uint8_t *data, uint8_t len, uint8_t received_checksum);

#endif
