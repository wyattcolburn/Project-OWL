#ifndef CDP_H
#define CDP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define SDUID 0x1234567891011123
#define DDUID 0x2019181716151413 //Dest addr 8 byte array
#define MUID 0x12345678    //Message unique ID 4 byte array
#define T 0x12 //Topic 1 byte 
#define DT 0x21//Duck type 1 byte
#define HC 0x99 //hop count 1 byte
#define DCRC 0x98765432 //Data section CRC

// Field/section length (in bytes)
#define PACKET_LENGTH 256
#define DUID_LENGTH 8
#define MUID_LENGTH 4
#define DATA_CRC_LENGTH 4
#define HEADER_LENGTH 27

#define MAX_DATA_LENGTH  PACKET_LENGTH - HEADER_LENGTH 
// Field/section offsets
#define SDUID_POS 0
#define DDUID_POS 8
#define MUID_POS 16
#define TOPIC_POS 20
#define DUCK_TYPE_POS 21
#define HOP_COUNT_POS 22
#define DATA_CRC_POS 23
#define DATA_POS HEADER_LENGTH // Data section starts immediately after header
			

typedef struct {
    uint8_t sduid[DUID_LENGTH];
    uint8_t dduid[DUID_LENGTH];
    uint8_t muid[MUID_LENGTH];
    uint8_t topic;
    uint8_t duckType;
    uint8_t hopCount;
    uint32_t dcrc;
    uint8_t data[MAX_DATA_LENGTH];
    size_t dataLength;
} CdpPacket;


void buffer_init(uint8_t*cdpBuffer, int cdpBuffer_len);
void initCdpPacket(CdpPacket * packet);
void generate_cdp(uint8_t *cdpBuffer, const uint8_t *dataBuffer, size_t dataLength);
void resetCdpPacket(CdpPacket *packet);
void decode_cdp(uint8_t *cdpBuffer, size_t bufferLength, CdpPacket * packet);
void reverse_bytes(uint8_t*dest, const void*src, size_t size);
#endif
