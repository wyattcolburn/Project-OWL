#include <lgpio.h>
#include <stdio.h>

#define SPI_DEV 0
#define SPI_CHANNEL 0
#define SPI_SPEED 9600
#define SPI_FLAGS 0

int main(){


	int chip_handle = lgGpiochipOpen(0);
	if (chip_handle >= 0){
		puts("chip opened");
	}
	else{
		printf("error opening chip %d\n", chip_handle);
	}

	//open spi port
	
	int spi_handle = lgSpiOpen(SPI_DEV, SPI_CHANNEL, SPI_SPEED, SPI_FLAGS);

	if (spi_handle >= 0) {
		puts("spi port opened");

	}
	else {
		printf("error:  %d\n", spi_handle);
	}

	/*char txBuffer[2] = {0xC0, 0x00};*/
	/*lgSpiWrite(spi_handle, txBuffer, sizeof(txBuffer));*/

// Define your opcode and buffer sizes
	char opcode[2] = {0xC0,0x05}; // Example opcode
	char rxBuffer[2]; // Adjust size as needed
	int count = sizeof(rxBuffer);

	// Send the opcode
	int writeResult = lgSpiWrite(spi_handle, opcode, 2);
	if (writeResult < 0) {
		printf("Failed to write to SPI device\n");
		lgSpiClose(spi_handle);
		return -1;
	}

	// Read the response
	int readResult = lgSpiRead(spi_handle, rxBuffer, count);
	if (readResult < 0) {
		printf("Failed to read from SPI device\n");
		lgSpiClose(spi_handle);
		return -1;
	}

	// Process the received data
	printf("Received %d bytes from SPI device:\n", readResult);
	for (int i = 0; i < readResult; i++) {
		printf("0x%02X ", rxBuffer[i]);
	}
	printf("\n");

    // Close the SPI device
    lgSpiClose(spi_handle);

    return 0;
}
