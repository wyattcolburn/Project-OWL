#include <stdio.h>
#include <lgpio.h>
#include "spi_loopback.h"

// Function prototypes
void lgpio_init(void);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);

int main() {
    puts("start");
    lgpio_init();

    // Open the SPI device, spi_handle is a handle
    int spi_handle = spiHandle(0, 0, 9600, 0);
    if (spi_handle < 0) {
        printf("SPI port failed to open: error code %d\n", spi_handle);
        return 1;
    }

    // Transmit to the SPI device
    const char txBuf[] = {0x11, 0x12, 0x13}; // Example data to send
    int writeCount = lgSpiWrite(spi_handle, txBuf, sizeof(txBuf));
    if (writeCount < 0) {
        printf("Failed to write to SPI device: error code %d\n", writeCount);
        lgSpiClose(spi_handle);
        return 1;
    }
    printf("Wrote %d bytes to SPI device\n", writeCount);

    
	
	// Read from the SPI device
    char rxBuff[3] = {0};
    int readCount = lgSpiRead(spi_handle, rxBuff, sizeof(rxBuff));
    if (readCount >= 0) {
        printf("Read %d bytes from SPI device\n", readCount);
        if (readCount == writeCount) {
            printf("Success: Received expected data\n");
            printBuffer(rxBuff, readCount);
        } else {
            printf("Error: Read count (%d) does not match write count (%d)\n", readCount, writeCount);
            printBuffer(rxBuff, readCount);
        }
    } else {
        printf("Failed to read from SPI device: error code %d\n", readCount);
    }

    // Close SPI device
    int closeStatus = lgSpiClose(spi_handle);
    if (closeStatus < 0) {
        printf("Failed to close SPI device: error code %d\n", closeStatus);
        return 1;
    }
    printf("SPI device closed successfully\n");

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

