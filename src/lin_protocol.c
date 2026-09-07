#include "../include/lin_protocol.h"

uint8_t LIN_ComputePID(uint8_t id)
{
    id &= 0x3F;

    uint8_t id0 = (id >> 0) & 0x01;
    uint8_t id1 = (id >> 1) & 0x01;
    uint8_t id2 = (id >> 2) & 0x01;
    uint8_t id3 = (id >> 3) & 0x01;
    uint8_t id4 = (id >> 4) & 0x01;
    uint8_t id5 = (id >> 5) & 0x01;

    uint8_t p0 = (id0 ^ id1 ^ id2 ^ id4) & 0x01;
    uint8_t p1 = (~(id1 ^ id3 ^ id4 ^ id5)) & 0x01;

    return (uint8_t)(id | (p0 << 6) | (p1 << 7));
}

uint8_t LIN_ComputeChecksum(uint8_t pid, const uint8_t *data, uint8_t len)
{
    uint16_t sum = pid;

    for (uint8_t i = 0; i < len; i++) {
        sum += data[i];
        if (sum > 0xFF) {
            sum -= 0xFF;
        }
    }

    return (uint8_t)(~sum & 0xFF);
}

uint8_t LIN_ValidateChecksum(uint8_t pid, const uint8_t *data, uint8_t len, uint8_t received_checksum)
{
    return (LIN_ComputeChecksum(pid, data, len) == received_checksum) ? 1 : 0;
}
