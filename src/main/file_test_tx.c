#include <stdio.h>
#include <stdlib.h>
#include "transmit.h"
#include "cdp.h"

char* readFile(const char *filename, size_t *size);
int chip_handle = 0;
int spi_handle = 0;

int main(){
	const char *filename = "example.txt";
    size_t size;
    char *buffer;
	
	buffer = readFile(filename, &size);
    if (buffer == NULL) {
        return 1;
    }
	// Print the buffer
    printf("File contents (%zu bytes):\n%s\n", size, buffer);
	puts("start transmit portion of code");


	chip_handle = lgpio_init();
	gpio_init(chip_handle);
	//put GPIO 26 as output for NSS-Reset(needs to be high)
    // Open the SPI device, spi_handle is a handle
    spi_handle = spiHandle(0, 0, 5000000, 0);
    if (spi_handle < 0) {
        printf("SPI port failed to open: error code %d\n", spi_handle);
        return 1;
    }
	puts("start cdp stuff");
	int cdpBuffer_len = size + 27;
	puts("1");
	// Allocate memory for cdpBuffer
    uint8_t *cdpBuffer = (uint8_t *)malloc(cdpBuffer_len);
    if (cdpBuffer == NULL) {
        printf("Failed to allocate memory for cdpBuffer\n");
        free(buffer); // Free the buffer read from the file
        return 1;
    }
    
    puts("2");
    buffer_init(cdpBuffer, cdpBuffer_len);
    puts("3");
    generate_cdp(cdpBuffer, (const uint8_t *)buffer, size);

    printf("Formatted Buffer: ");
    for (int i = 0; i < HEADER_LENGTH + size; i++) {
        printf("%02X ", cdpBuffer[i]);
    }
	puts("hi");
	send_packet(cdpBuffer, cdpBuffer_len);
	puts("5");
	return 0;
}


// Function to read the contents of a file and return it as a buffer
char* readFile(const char *filename, size_t *size) {
    FILE *file;
    char *buffer;
    size_t fileSize;

    // Open the file for reading
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer
    buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer
    fread(buffer, 1, fileSize, file);

    // Null-terminate the buffer
    buffer[fileSize] = '\0';

    // Close the file
    fclose(file);

    // Set the size output parameter
    *size = fileSize;

    return buffer;
}

