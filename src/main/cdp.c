#include <stdio.h>
#include <string.h>
#include "cdp.h"
/*
|0       |8       |16  |20|21|22|23  |27                                   255|
|        |        |    |  |  |  |    |                                        |
+--------+--------+----+--+--+--+----+----------------------------------------+
| SDUID  | DDUID  |MUID|T |DT|HC|DCRC|                 DATA                   |
|        |        |    |  |  |  |    |            (max 229 bytes)             |
+--------+--------+----+--+--+--+----+----------------------------------------+

SDUID:     08  byte array          - Source Device Unique ID
DDUID:     08  byte array          - Destination Device Unique ID
MUID:      04  byte array          - Message unique ID
T   :      01  byte value          - Topic (topic 0..15 are reserved for internal use)
DT  :      01  byte value          - Duck Type
HC  :      01  byte value          - Hop count (the number of times the packet was relayed)
DCRC:      04  byte value          - Data section CRC
DATA:      229 byte array          - Data payload (e.g sensor read, text,...)
*/

//assume that cdpBuffer has already been created with size already defined
void buffer_init(uint8_t * cdpBuffer, int cdpBuffer_len){

	memset(cdpBuffer, 0, cdpBuffer_len);
}
void initCdpPacket(CdpPacket *packet) {
    uint64_t sduid = SDUID;
    uint64_t dduid = DDUID;
    uint32_t muid = MUID;
	
    memcpy(packet->sduid, &sduid, DUID_LENGTH);
    memcpy(packet->dduid, &dduid, DUID_LENGTH);
    memcpy(packet->muid, &muid, MUID_LENGTH);
    packet->topic = T;
    packet->duckType = DT;
    packet->hopCount = HC; 
    packet->dcrc = DCRC;
    packet->dataLength = 0;
    memset(packet->data, 0, MAX_DATA_LENGTH);

}



void generate_cdp(uint8_t *cdpBuffer, const uint8_t *dataBuffer, size_t dataLength) {
	// Ensure data length does not exceed max data length
	if (dataLength > MAX_DATA_LENGTH) {
		dataLength = MAX_DATA_LENGTH;
	}
	// Copy default values into the buffer
	uint64_t sduid = SDUID;
	uint64_t dduid = DDUID;
	uint32_t muid = MUID;
	uint8_t topic = T;
	uint8_t duckType = DT;
	uint8_t hopCount = HC;
	uint32_t dcrc = DCRC;

	reverse_bytes(&cdpBuffer[SDUID_POS], &sduid, DUID_LENGTH);
	reverse_bytes(&cdpBuffer[DDUID_POS], &dduid, DUID_LENGTH);
	reverse_bytes(&cdpBuffer[MUID_POS], &muid, MUID_LENGTH);
	cdpBuffer[TOPIC_POS] = topic;
	cdpBuffer[DUCK_TYPE_POS] = duckType;
	cdpBuffer[HOP_COUNT_POS] = hopCount;
	reverse_bytes(&cdpBuffer[DATA_CRC_POS], &dcrc, DATA_CRC_LENGTH);

	// Copy the data into the buffer
	memcpy(&cdpBuffer[DATA_POS], dataBuffer, dataLength);
}
void resetCdpPacket(CdpPacket *packet) {
    initCdpPacket(packet);
}

void reverse_bytes(uint8_t* dest, const void* src, size_t size) {
    const uint8_t* src_bytes = (const uint8_t*)src;
    for (size_t i = 0; i < size; i++) {
        dest[i] = src_bytes[size - 1 - i];
    }
}void decode_cdp(uint8_t *cdpBuffer, size_t bufferLength, CdpPacket * packet){

	if (bufferLength < HEADER_LENGTH) {
		puts("receive message has no data");
		return;
	}
    memcpy(packet->sduid, &cdpBuffer[SDUID_POS], DUID_LENGTH);
    memcpy(packet->dduid, &cdpBuffer[DDUID_POS], DUID_LENGTH);
    memcpy(packet->muid, &cdpBuffer[MUID_POS], MUID_LENGTH);
    packet->topic = cdpBuffer[TOPIC_POS];
    packet->duckType = cdpBuffer[DUCK_TYPE_POS];
    packet->hopCount = cdpBuffer[HOP_COUNT_POS];
    packet->dcrc = *(uint32_t*)&cdpBuffer[DATA_CRC_POS];
    packet->dataLength = bufferLength - HEADER_LENGTH;
    memcpy(packet->data, &cdpBuffer[DATA_POS], packet->dataLength);
}
