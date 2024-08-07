#include <stdio.h>
#include <stdlib.h>
#include "transmit.h"
#include "cdp.h"

int chip_handle = 0;
int spi_handle = 0;
void printCdpPacket(const CdpPacket *packet);

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
	puts("reset");
	CdpPacket packet;
	puts("init cdp packet");

	initCdpPacket(&packet);
puts("clear cdp packet");
	char rx_pkt[229] = {};

	rx_mode_attempt(rx_pkt);
	int payload_len = strlen(rx_pkt);
	for(int i = 0; i < payload_len; i++)
	{
	printf("%c", rx_pkt[i]);
	}


	decode_cdp((uint8_t *) rx_pkt, payload_len+27, &packet);

	
	printCdpPacket(&packet);	


	return 0;
}

void printCdpPacket(const CdpPacket *packet) {
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
    printf("DCRC: %08X\n", packet->dcrc);

    printf("Data: ");
    for (size_t i = 0; i < packet->dataLength; ++i) {
        printf("%c", packet->data[i]);
    }
    printf("\n");
}
