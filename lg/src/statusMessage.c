#include <stdio.h>
#include <lgpio.h>
#include "statusMessage.h"


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
int busyCheck(void);

int chip_handle = 0;
int main() {
    puts("start");
    chip_handle =  lgpio_init();
	gpio_init(chip_handle);
	//put GPIO 26 as output for NSS-Reset(needs to be high)

    // Open the SPI device, spi_handle is a handle
    int spi_handle = spiHandle(0, 0, 5000000, 0);
    if (spi_handle < 0) {
        printf("SPI port failed to open: error code %d\n", spi_handle);
        return 1;
    }
	
    // Example opcode for status message
     // Replace with actual opcode
    char response[2] = {0}; // Buffer to hold the response

    int result = getStatus(spi_handle, GET_STATUS_OP, response, sizeof(response));
    if (result < 0) {
        printf("Failed to get status: error code %d\n", result);
        lgSpiClose(spi_handle);
        return 1;
    }
    printf("Received status: ");
    printBuffer(response, sizeof(response));

    // Close SPI device
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
	if (busyCheck() == READY) {
	//send status OP Code
		return lgSpiXfer(spi_handle, txBuf, response, response_len);
	}
	return 0;
}

void gpio_init(int chip_handle){

	int sx_nreset =  lgGpioClaimOutput(chip_handle,0, SX_NRESET_PIN, HIGH); //init high
	if (sx_nreset <0 ) {
		printf("Failure to init GPIO 26: error code %d\n", sx_nreset);
	}
	else {
		printf("NSS INIT success\n");
	}

	int busy = lgGpioClaimInput(chip_handle, 0, 16); //GPIO 16 input

	if (busy <0 ) {
		printf("Failure to init GPIO 26: error code %d\n", busy);
	}
	else {
		printf("BUSY INIT success\n");
	}

	int busyCheck1 = busyCheck();
	if (busyCheck1 == LOW) {
		printf("busy line low, is ready");
	}
	else if (busyCheck1 == HIGH) {
		printf("busy line high, not ready");
	}
	else {
		printf("error code %d\n", busyCheck1);
	}
		
}




int busyCheck(void){
	//this function checks the busy pin on the Sx1262, busy pin must be low
	//for the chip to receive commands
	//Function returns 0, if ready, and returns 1 if busy
	//MACROS defines as 0 = READY, 1 = BUSY	
	int busyStatus= lgGpioRead(chip_handle, BUSY_PIN); 
	if (busyStatus == LOW) {
  	return READY;
	}
	return  BUSY; //return the stat
}

void tx_mode(){
  /*
  1) Go into STDBY MODE, using SetStandby 
  2) Set packet type--SetPacketType
  3) Define  the RF frequency--SetRfFrequency
  4) Define power ampl confi--SetPaConfig()
  5) Define output power and ramping time with command 
        SetTxParams()
  6) Define where the data payload will stored with command 
      SetBufferBaseAddress()
  7) Send payload to data buffer--> WriteBuffer()
  8) Define modulation paramter according to chosen protocol 
      with command SetModulationParams()

  9) Define frame format with SetPacketParams()^2
  10) Map TxDone IRQ to a DIO (DIO 1,2,or 3)
  11) Define a sync value: use command WriteReg() to write value to register via direct access
  12) Set circuit in transmitter mode to start tranmsiion SetTx(), use param to enable Timeout
  13) Wait for IRQ TxDone  or timeout-> chip once finished will go into STDBY_RC mode
  14) Clear IRQ TxDone Flag
  */


  
  
}


int sendCommand(int spi_handle, char command, int command_len){

  




}
