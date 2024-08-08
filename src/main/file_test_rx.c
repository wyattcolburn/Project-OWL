#include <stdio.h>
#include <stdlib.h>
#include "sx1262.h"
#include "cdp.h"
#include <ctype.h>
int chip_handle = 0;
int spi_handle = 0;
void printCdpPacket(const CdpPacket *packet);
void remove_spaces(uint8_t *buffer);
void processHexPacket(const char *input , unsigned char * output, int *outputLen);
void writeCdpPacket(const CdpPacket *packet);
int main(){
chip_handle = lgpio_init();
	gpio_init(chip_handle);
	//put GPIO 26 as output for NSS-Reset(needs to be high)
    // Open the SPI device, spi_handle is a handle
    spi_handle = spiHandle(0, 0, 5000000, 0);
    if (spi_handle < 0) {
        printf("SPI port failed to open: error code %d\n", spi_handle);
        return 1;
    }
	factoryReset();
	
	CdpPacket packet;

	initCdpPacket(&packet);
	uint8_t rx_pkt[229] = {};

	rx_mode_attempt(rx_pkt);
	printf("message received\n");

	int payload_len = sizeof(rx_pkt);

	unsigned char processedData[sizeof(rx_pkt)/2];
	int processedDataLen;

	processHexPacket((char *)rx_pkt, processedData, &processedDataLen);
	
	printf("Processed Data:\n");
    for (int i = 0; i < processedDataLen; i++) {
        printf("%02X", processedData[i]);
    }
    printf("\n");


	decode_cdp(processedData, processedDataLen, &packet);
	
	
	printCdpPacket(&packet);
	writeCdpPacket(&packet);
	return 0;
}
void remove_spaces(uint8_t *buffer) {
    int i, j = 0;
    int length = sizeof(buffer);

    for (i = 0; i < length; i++) {
        if (buffer[i] != ' ') {
            buffer[j++] = buffer[i];
        }
    }
    buffer[j] = '\0'; // Null-terminate the modified buffer
}
void printCdpPacket(const CdpPacket *packet) {
	printf("\n");
    printf("SDUID: ");
    for (int i = 0; i < DUID_LENGTH; ++i) {
        printf("%02X ", packet->sduid[i]);
    }
    printf("\n");

    printf("DDUID: ");
    for (int i = 0; i < DUID_LENGTH; ++i) {
        printf("%02X ", packet->dduid[i]);
    }
    printf("\n");

    printf("MUID: ");
    for (int i = 0; i < MUID_LENGTH; ++i) {
        printf("%02X ", packet->muid[i]);
    }
    printf("\n");

    printf("Topic: %02X\n", packet->topic);
    printf("Duck Type: %02X\n", packet->duckType);
    printf("Hop Count: %02X\n", packet->hopCount);
    printf("DCRC: %02X\n", packet->dcrc);

    printf("Data: ");
    for (size_t i = 0; i < packet->dataLength ; ++i) {
        printf("%c", packet->data[i]);
    }
    printf("\n");
}
void writeCdpPacket(const CdpPacket *packet) {
    FILE *file = fopen("output.txt", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    fprintf(file, "\n");
    fprintf(file, "SDUID: ");
    for (int i = 0; i < DUID_LENGTH; ++i) {
        fprintf(file, "%02X ", packet->sduid[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "DDUID: ");
    for (int i = 0; i < DUID_LENGTH; ++i) {
        fprintf(file, "%02X ", packet->dduid[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "MUID: ");
    for (int i = 0; i < MUID_LENGTH; ++i) {
        fprintf(file, "%02X ", packet->muid[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "Topic: %02X\n", packet->topic);
    fprintf(file, "Duck Type: %02X\n", packet->duckType);
    fprintf(file, "Hop Count: %02X\n", packet->hopCount);
    fprintf(file, "DCRC: %02X\n", packet->dcrc);

    fprintf(file, "Data: ");
    for (size_t i = 0; i < packet->dataLength; ++i) {
        fprintf(file, "%c", packet->data[i]);
    }
    fprintf(file, "\n");

    fclose(file);
}
void processHexPacket(const char *input, unsigned char *output, int *outputLen) {
    int i = 0, j = 0;
    char temp[3] = {0}; // Temporary buffer to hold a hex pair

    while (input[i] != '\0') {
        if (isspace(input[i])) {
            i++;
            continue;
        }

        // Copy the next two characters as a hex pair
        temp[0] = input[i++];
        temp[1] = input[i++];

        // Convert the hex pair to an integer
        output[j++] = (unsigned char)strtol(temp, NULL, 16);
    }

    *outputLen = j;
}


