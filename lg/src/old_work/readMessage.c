#include <stdio.h>
#include <lgpio.h>
#include "spi_loopback.h"

#define BUFFER_LEN 10
void lgpio_init(void);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);

int main(){
	puts("start");
	lgpio_init(); //start gpio chip
	
	int spi_handle = spiHandle(0,0,9600,0); //get a handle for the spi 
	for (int i =0; i <3; i++) {	
		char rxBuf[BUFFER_LEN] ={0}; //rxBuffer filled with 0
		int readStatus = lgSpiRead(spi_handle,rxBuf, BUFFER_LEN);
		printBuffer(rxBuf, BUFFER_LEN);
	}
	return 0;
}

void lgpio_init(void) {
    uint8_t h;
    h = lgGpiochipOpen(0);

    if (h >= 0) {
        puts("GPIO chip opened");
    } else {
        puts("Failed to open GPIO chip");
    }
}
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag) {
    int spiOpenVal = lgSpiOpen(spiDev, spiChannel, spiBaud, spiFlag);
    if (spiOpenVal >= 0) {
        puts("SPI port open success");
    } else {
        puts("SPI port failed to open");
    }
    return spiOpenVal;
}

void printBuffer(const char *buffer, int len) {
    // This function takes a buffer and outputs it byte by byte
    for (int i = 0; i < len; i++) {
        printf("0x%02x ", (unsigned char)buffer[i]);
    }
    printf("\n");
	
}
