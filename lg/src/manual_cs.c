#include <stdio.h>
#include <lgpio.h>
#include <math.h>
#include "statusMessage.h"
#include "sx1262x_defs_custom.h"

#define GET_STATUS_OP                   UINT8_C(0xC0)
#define GET_STATS_OP                    UINT8_C(0x10)
#define GET_RST_STATS_OP                UINT8_C(0x00)
/*Pin Connections:*/

/*LoRa Shield -> Raspberry Pi*/
/*J1: */
	/*BUSY 4*/
	/*DIO1 6*/
	/*NSS 8*/

/*J2:*/
	/*ANT SW 1*/
	/*D9 2 */
	/*MOSI 4->19 (white cable)*/
	/*MISO 5 ->21 (black cable)*/
	/*SCLK 6 ->23 (grey cable)*/
	/*GND 7*/

/*J3:*/
	/*GND 6 & 7*/
	/*VCC MBED, VDD 3V3, VDD_RADIO 4*/

/*J4:	*/
	/*SX RESET 1*/
	/*VDD 3V3 /GND 2*/
	/*GND 3*/
	/*VCC (XTAL) 4 -> Pin 1*/
	/*A5 5*/
	/*A6 6*/


// Function prototypes
int lgpio_init(void);
void gpio_init(int chip_handle);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);
int getStatus(int spi_handle, char opcode, char *response, int response_len);
void wait_on_busy(void);

int chip_handle = 0;
int spi_handle = 0;
int main() {
    puts("start");
    chip_handle =  lgpio_init();
	gpio_init(chip_handle);
	//put GPIO 26 as output for NSS-Reset(needs to be high)

    // Open the SPI device, spi_handle is a handle
    spi_handle = spiHandle(0, 0, 5000000, 0);
    if (spi_handle < 0) {
        printf("SPI port failed to open: error code %d\n", spi_handle);
        return 1;
    }
	/*uint8_t command = 0xC0;*/
	/*uint8_t data = 0x00;*/
	/*int command_len = 1;*/
	/*int result = send_read_command(spi_handle, command, &data, command_len);*/
	
    /*if (result) {*/
        /*printf("Command sent successfully\n");*/
    /*} else {*/
        /*printf("Failed to send command\n");*/
    /*}*/
	uint8_t tx_clamp_config_val;
	read_registers(REG_TX_CLAMP_CONFIG, &tx_clamp_config_val, 1);
	printf("0x%02X\n", tx_clamp_config_val);
	 /*Example opcode for status message*/
	  /*Replace with actual opcode*/
    /*char response[2] = {0}; // Buffer to hold the response*/

    /*int result = getStatus(spi_handle, GET_STATUS_OP, response, sizeof(response));*/
    /*if (result < 0) {*/
        /*printf("Failed to get status: error code %d\n", result);*/
        /*lgSpiClose(spi_handle);*/
        /*return 1;*/
    /*}*/
    /*printf("Received status: ");*/
    /*printBuffer(response, sizeof(response));*/

    // Close SPI device
	//
    int closeStatus = lgSpiClose(spi_handle);
    if (closeStatus < 0) {
        printf("Failed to close SPI device: error code %d\n", closeStatus);
        return 1;
    }
    printf("SPI device closed successfully\n");

    return 0;
}

int lgpio_init(void) {
    uint8_t h;
    h = lgGpiochipOpen(0);

    if (h >= 0) {
        puts("GPIO chip opened");
    } else {
        puts("Failed to open GPIO chip");
    }
	return h;
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

int getStatus(int spi_handle, char opcode, char *response, int response_len) {

	//building the buffer
    char txBuf[response_len];
    txBuf[0] = opcode; // Set the opcode
    for (int i = 1; i < response_len; i++) {
        txBuf[i] = 0x00; // Fill the rest with dummy data
    }
    printf("Tx Buffer: ");
	printBuffer(txBuf, response_len);
	//first check if busy line is low
	wait_on_busy();
	//send status OP Code
		return lgSpiXfer(spi_handle, txBuf, response, response_len);
	
}


void gpio_init(int chip_handle){

	int sx_nreset =  lgGpioClaimOutput(chip_handle,0, SX_NRESET_PIN, HIGH); //init high
	(sx_nreset >= 0 ) ? puts("sx_nreset init") : puts("sx_nreset fail");
	

	int busy = lgGpioClaimInput(chip_handle, 0, 16); //GPIO 16 input
	(busy >= 0) ? puts("busy init") : puts("busy fail");
	

	int txIRQ = lgGpioClaimInput(chip_handle, 0, 6); //gpio 6 input
	
	(txIRQ >= 0) ? puts("tx init") : puts("tx fail");
	
	int chip_select = lgGpioClaimOutput(chip_handle,0, CS_PIN, HIGH);
	/*(chip_select >= 0) ? puts("CS init") : ("CS failed");*/

	(chip_select >= 0 ) ? puts("cs init") : puts("cs fail");
}

void nss_select(){
	/*
	This function is used for manual control of the CS pin (external gpio,
	not the chip one). This function pulls pin low, should be down before 
	spi transactions
	*/
	int nss_low = lgGpioWrite(chip_handle, CS_PIN, LOW);
	int nss_status = lgGpioRead(chip_handle, CS_PIN);
	
	//blocking code to make sure nss goes low
	/*while (nss_status == HIGH) {*/
		/*SLEEP_MS(1);*/
		/*nss_status = lgGpioRead(chip_handle, CS_PIN);*/
	/*}*/
	puts("CS LOW");

}
void nss_deselect(){
	int nss_high = lgGpioWrite(chip_handle, CS_PIN, HIGH);
	
	int nss_status = lgGpioRead(chip_handle, CS_PIN);
	
	//blocking code to make sure nss goes high
	/*while (nss_status == LOW) {*/
		/*SLEEP_MS(1);*/
		/*nss_status = lgGpioRead(chip_handle, CS_PIN);*/
	/*}*/
	puts("CS HIGH");
}

void wait_on_busy(void){
	//this function checks the busy pin on the Sx1262, busy pin must be low
	//for the chip to receive commands, delays until pin goes low
	
	int busyStatus= lgGpioRead(chip_handle, BUSY_PIN); 
	while (busyStatus == HIGH) {
		SLEEP_MS(1);
	}
}

void wait_on_TX_IRQ(void) {

	//waits for TX IRQ to go low, means tx done and clears IRQ flag
	
	int txStatus= lgGpioRead(chip_handle, BUSY_PIN); 
	while (txStatus == HIGH) {
		SLEEP_MS(1);
	}
}

/*void clear_tx_irq(){*/

	/*//not sure how to do this? Think I am clearing all the status*/
	/*uint8_t NOP_buffer[2] = {NO_OPERATION, NO_OPERATION};*/
	/*uint16_t  tx_irq_status = read_registers(GET_IRQ_STATUS_OP, NOP_buffer, 2);*/
	
	/*tx_irq_status &= ~TX_DONE_MASK;*/

	/*write_registers(CLEAR_IRQ_STATUS_OP, (uint8_t *)tx_irq_status, 1);*/
/*}	*/
uint8_t write_registers(uint16_t reg_addr, uint8_t * data, uint8_t len) {
	//ISSUES MIGHT ARISE WITH NSS going high and low through multiple spi operations
	uint8_t status;
	uint8_t cmd_addr[3] = {WRITE_REG_OP, (uint8_t) ((reg_addr >> 8) & (0x00FF)),
						  (uint8_t) (reg_addr & 0x00FF)};
	uint8_t spiWrite = lgSpiWrite(spi_handle, (const char *) cmd_addr, 3);
	uint8_t spiWrite2 = lgSpiWrite(spi_handle, (const char *) data, len - 1);
	uint8_t spiRead_Write = lgSpiXfer(spi_handle, (const char *) data + (len -1),(char *) &status, 1);

	wait_on_busy();

	return status;

}
uint8_t read_registers(uint16_t reg_addr, uint8_t* data, uint8_t len){
	puts("reading registers");

	uint8_t status;
	uint8_t cmd_addr[3] = {READ_REG_OP, (uint8_t) ((reg_addr >> 8) & (0x00FF)),
						  (uint8_t) (reg_addr &0x0FF)};

	nss_select(); //should go low
	int spiWrite = lgSpiWrite(spi_handle, (const char *) cmd_addr, 3); 
	(spiWrite >= 0) ? puts("spi write read") : puts("cannot write");

	int spiRead_Write = lgSpiXfer(spi_handle, (const char *) NO_OPERATION, &status, 1);//dif from quad, 
	int spiRead = lgSpiRead(spi_handle, (char *) data, len); //reads data into paramters
	nss_deselect(); //nss goes back high
	wait_on_busy();

	return status;
}


//do we want to have this return a value if the message goes through??
void sendCommand(int spi_handle, uint8_t opcode, uint8_t* data, int command_len){
	//this function sends commands does not look for their resposne
	//create a buffer first of uint8_t data type
	uint8_t tx_buffer[1+command_len];

	tx_buffer[0] = opcode;
	if (command_len > 0) {
		
		for (int bufferIndex = 1; bufferIndex < (command_len + 1); bufferIndex ++) {
			tx_buffer[bufferIndex] = data[bufferIndex - 1];
		}
	}
	int Status = lgSpiWrite(spi_handle, (const char *) tx_buffer, command_len + 1);
	if (opcode != SET_SLEEP_OP) {
		wait_on_busy();
	}
}



int send_read_command(int spi_handle, uint8_t command, uint8_t* data, int command_len) {
    // Create a buffer with the size of command + data
    uint8_t tx_buffer[1 + command_len];
    uint8_t rx_buffer[1 + command_len]; // Buffer to receive the response

    // Place the command at the start of the buffer
    tx_buffer[0] = command;

    // Copy the data into the buffer starting from index 1
    for (int bufferIndex = 1; bufferIndex < (command_len + 1); bufferIndex++) {
        tx_buffer[bufferIndex] = data[bufferIndex - 1];
    }

    // Initialize the rx_buffer to ensure it is empty before the transfer
    for (int i = 0; i < (1 + command_len); i++) {
        rx_buffer[i] = 0;
    }

    // Perform the SPI transfer
    int Status = lgSpiXfer(spi_handle, (const char *)tx_buffer, (char *)rx_buffer, command_len + 1);

    if (Status >= 0) {
        // If transfer is successful, process the received data if needed
        // For example, print the received data
        for (int i = 0; i < Status; i++) {
            printf("Received byte %d: %02x\n", i, rx_buffer[i]);
        }
        return 1;
    }
    return 0;
}

