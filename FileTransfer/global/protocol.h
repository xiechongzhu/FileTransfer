#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

static const uint32_t MAGIC_NUMBER = 0x12345678;

static const uint32_t CMD_HANDSHANK_REQ = 1;
static const uint32_t CMD_HANDSHANK_RESP = 2;
static const uint32_t CMD_FILE_START = 3;
static const uint32_t CMD_FILE_END = 4;
static const uint32_t CMD_FILE_DATA = 5;

#pragma pack(push, 1)

typedef struct
{
    uint32_t magic_number;
    uint32_t cmd;
    uint32_t body_length;
    uint8_t  data[];
}Packet;

#pragma pack(pop)

#endif // PROTOCOL_H
