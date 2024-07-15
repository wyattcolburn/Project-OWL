#include <stdio.h>
#include <stdlib.h>
#include <lgpio.h>
#include <unistd.h>

#define SPI_BUS 0
#define SPI_CS 0
#define SPI_SPEED 500000  // 500 kHz
#define READ_STATUS_CMD 0xC0  // Replace with the actual command from the datasheet

int init_spi() {
    int handle = lgGpiochipOpen(0);
    if (handle < 0) {
        fprintf(stderr, "Failed to open GPIO chip\n");
        return -1;
    }
    int spi_handle = lgSpiOpen(SPI_BUS, SPI_CS, SPI_SPEED, 0);
    if (spi_handle < 0) {
        fprintf(stderr, "Failed to open SPI bus\n");
        lgGpiochipClose(handle);
        return -1;
    }
    return spi_handle;
}

void send_command(int spi_handle, uint8_t command) {
    lgSpiWrite(spi_handle, &command, 1);
}

uint8_t* read_status(int spi_handle) {
    static uint8_t status[2];
    send_command(spi_handle, READ_STATUS_CMD);
    usleep(100000);  // Wait for the command to process
    lgSpiXfer(spi_handle, NULL, status, 2);  // Read two bytes of status
    return status;
}

int main() {
    int spi_handle = init_spi();
    if (spi_handle < 0) {
        return EXIT_FAILURE;
    }

    uint8_t* status = read_status(spi_handle);
    printf("Status: %02X %02X\n", status[0], status[1]);

    lgSpiClose(spi_handle);
    return EXIT_SUCCESS;
}

