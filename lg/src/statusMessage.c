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
	uint8_t command = 0xC0;
	uint8_t data = 0x00;
	int command_len = 1;
	int result = send_read_command(spi_handle, command, &data, command_len);
	
    if (result) {
        printf("Command sent successfully\n");
    } else {
        printf("Failed to send command\n");
    }
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
	
	/*if (sx_nreset <0 ) {*/
		/*printf("Failure to init GPIO 26: error code %d\n", sx_nreset);*/
	/*}*/
	/*else {*/
		/*printf("NSS INIT success\n");*/
	/*}*/

	int busy = lgGpioClaimInput(chip_handle, 0, 16); //GPIO 16 input
	(busy >= 0) ? puts("busy init") : puts("busy fail");
	
	/*if (busy <0 ) {*/
		/*printf("Failure to init GPIO 26: error code %d\n", busy);*/
	/*}*/
	/*else {*/
		/*printf("BUSY INIT success\n");*/
	/*}*/

	int txIRQ = lgGpioClaimInput(chip_handle, 0, 6); //gpio 6 input
	
	(txIRQ >= 0) ? puts("tx init") : puts("tx fail");
	
	/*int busyCheck1 = busyCheck();*/
	/*if (busyCheck1 == LOW) {*/
		/*printf("busy line low, is ready");*/
	/*}*/
	/*else if (busyCheck1 == HIGH) {*/
		/*printf("busy line high, not ready");*/
	/*}*/
	/*else {*/
		/*printf("error code %d\n", busyCheck1);*/
	/*}*/
		
}




void wait_on_busy(void){
	//this function checks the busy pin on the Sx1262, busy pin must be low
	//for the chip to receive commands, delays until pin goes low
	
	int busyStatus= lgGpioRead(chip_handle, BUSY_PIN); 
	while (busyStatus == HIGH) {
		SLEEP_MS(1);
	}
}

void tx_mode(void){
  /*
  1) Go in to STDBY MODE, using SetStandby 
  2) Set packet type--SetPacketType
  3) Define  the RF frequency--SetRfFrequency
  4) Define power ampl confi--SetPaConfig(), step 4 done in setTxParams
  5) Define output power and ramping time with command 
        SetTxParams()
  6) Define where the data payload will stored with command 
      SetBufferBaseAddress()
  7) Send payload to data buffer--> WriteBuffer()
  8) Define modulation paramter according to chosen protocol 
      with command SetModulationParams()

  9) Define frame format with SetPacketParams()^2
  10) Map TxDone IRQ to a DIO (DIO 1,2,or 3)
  11) Define a sync value: use command WriteReg() to write value to register via direct access--NOt sure if this needed, Kev does not do this
  12) Set circuit in transmitter mode to start tranmsiion SetTx(), use param to enable Timeout//
  13) Wait for IRQ TxDone  or timeout-> chip once finished will go into STDBY_RC mode
  14) Clear IRQ TxDone Flag--not sure 
  */
	
	uint8_t data[5] = {1,2,3,4,5};
	uint8_t data_len = 5;

	set_standby_mode();
	set_packet_type(LORA_PKT_TYPE);
	set_rf_frequency(915000000); //idk what frequency to run
	set_tx_params(22, SET_RAMP_3400U);
	set_buffer_base_addr(0x00, 0x00); //why both zeros? 
	write_buffer(0x00, data, data_len); //write to buffer
										
	config_modulation_params(LORA_SF_7, LORA_BW_500, LORA_CR_4_5, 0x00); //low data optimazation off
	config_packet_params(12, PKT_EXPLICIT_HDR, data_len, PKT_CRC_OFF, STD_IQ_SETUP);

	set_dio_irq_params(0x03FF, 0x03FF, 0x0000, 0x000); //we are not using DIO2 or 3 yet so set them to zero
	set_tx_mode(0x00); //why no timeout??
	wait_on_TX_IRQ();
	puts("tx has been finished");
	//need to clear the irq flag??


	



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
void set_standby_mode(){
	//0 for RC
	//1 for XOCX
	sendCommand(spi_handle, SET_STANDBY_OP, 0, 1); //step 1
}
void set_packet_type(uint8_t packet_type){
	if(packet_type == LORA_PKT_TYPE ||
        packet_type == GFSK_PKT_TYPE)
    {
		sendCommand(spi_handle, SET_PACKET_TYPE_OP, &packet_type, 1);
	}
}

void set_rf_frequency(uint32_t frequency_mhz){
	uint8_t buff[4];
    uint32_t sx_freq = 0;

    // Calculate RF frequency
    sx_freq = (uint32_t)((double) frequency_mhz / (double) FREQ_STEP);

    buff[0] = (uint8_t) ((sx_freq >> 24) & 0xFF);
    buff[1] = (uint8_t) ((sx_freq >> 16) & 0xFF);
    buff[2] = (uint8_t) ((sx_freq >> 8) & 0xFF);
    buff[3] = (uint8_t) (sx_freq & 0xFF);

	sendCommand(spi_handle, SET_RF_FREQUENCY_OP, buff, 4);

}

void set_pa_config(uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_sel){
//page 77 of data sheet
	if(pa_duty_cycle > 0x04) //this is above the spec, should not happen 
        return; // Do nothing in this case

    if(hp_max > 0x07) // Clamp to the recommended max as per datasheet
        hp_max = 0x07;

    if(device_sel != 0x00 && device_sel != 0x01)
        return; // Do nothing in this case

    uint8_t pa_config_buff[4] = {pa_duty_cycle, hp_max, device_sel, 0x01};
	sendCommand(spi_handle, SET_PA_CONFIG_OP, pa_config_buff, 4);
}

void set_tx_params(int8_t power, uint8_t ramp_time){

	uint8_t tx_clamp_config_val;
	read_registers(REG_TX_CLAMP_CONFIG, &tx_clamp_config_val, 1);
	tx_clamp_config_val |= 0x1E;
    write_registers(REG_TX_CLAMP_CONFIG, &tx_clamp_config_val, 1);


	set_pa_config(0x04, 0x07, SX1262_DEV_TYPE);
	if (power < -9 ){
		power = -9;
	}
	else if (power > 22) {
		power = 22;
	}
	uint8_t ocp_setting = OCP_SX1262_140_MA;
	write_registers(REG_OCP_CONFIG, (char *) &ocp_setting, 1); //whhy &
	
	if (ramp_time > SET_RAMP_3400U) {
		ramp_time = SET_RAMP_3400U;
	}
	uint8_t tx_config_buff[2] = {power, ramp_time};
	sendCommand(spi_handle, SET_TX_PARAM_OP, tx_config_buff, 2);
}
void set_buffer_base_addr(uint8_t tx_base_addr, uint8_t rx_base_addr){
	uint8_t addr_buff[2] = {tx_base_addr, rx_base_addr};

	sendCommand(spi_handle, SET_BUFFER_BASE_ADDR_OP, addr_buff, 2);
}
uint8_t write_buffer(uint8_t offset, uint8_t* data, uint16_t len){

	uint8_t status;
	uint8_t cmd_offset[2] = {WRITE_BUF_OP, offset};

	int spiWrite = lgSpiWrite(spi_handle, (const char *) cmd_offset, 2);
	int spiWrite2 = lgSpiWrite(spi_handle, (const char *) data, len - 1);
	int spiReadWrite = lgSpiXfer(spi_handle, (const char *) data + (len +1 ), (char *) &status, 1);

	wait_on_busy();

	return status;
}

void config_modulation_params(uint8_t spreading_factor, uint8_t bandwidth, uint8_t coding_rate, uint8_t low_data_rate_opt){

	uint8_t mod_params[4] = {spreading_factor, bandwidth, coding_rate, low_data_rate_opt};

	sendCommand(spi_handle, SET_MOD_PARAM_OP, mod_params, 4);

}

void config_packet_params(uint8_t preamble_length, uint8_t header_type, uint8_t payload_length, uint8_t crc_type, uint8_t invert_iq) {

	uint8_t packet_param_buff[6];

	packet_param_buff[0] = (uint8_t)((preamble_length >>8 ) & 0x00FF);
	packet_param_buff[1] = (uint8_t)((preamble_length & 0x00FF));
	packet_param_buff[2] = (header_type == IMPLICIT_HDR) ? IMPLICIT_HDR : EXPLICIT_HDR;
	packet_param_buff[3] = payload_length;
	packet_param_buff[4] = (crc_type == CRC_ON) ? CRC_ON : CRC_OFF;
	packet_param_buff[5] = (invert_iq == INV_IQ_SETUP) ? INV_IQ_SETUP : STD_IQ_SETUP;

	sendCommand(spi_handle, SET_PACKET_PARAMS_OP, packet_param_buff, 6);
}

void set_dio_irq_params(uint16_t irq_mask, uint16_t dio1_mask, uint16_t dio2_mask,
						uint16_t dio3_mask) {

	uint8_t irq_dio_buff[8];

	 // IRQ Mask
    irq_dio_buff[0] = (uint8_t) ((irq_mask >> 8) & 0x00FF);
    irq_dio_buff[1] = (uint8_t) (irq_mask & 0x00FF);

    // DIO1 Mask
    irq_dio_buff[2] = (uint8_t) ((dio1_mask >> 8) & 0x00FF);
    irq_dio_buff[3] = (uint8_t) (dio1_mask & 0x00FF);

    // DIO2 Mask
    irq_dio_buff[4] = (uint8_t) ((dio2_mask >> 8) & 0x00FF);
    irq_dio_buff[5] = (uint8_t) (dio2_mask & 0x00FF);
    
    // DIO3 Mask
    irq_dio_buff[6] = (uint8_t) ((dio3_mask >> 8) & 0xFF);
    irq_dio_buff[7] = (uint8_t) (dio3_mask & 0x00FF);

    sendCommand(spi_handle, SET_DIO_IRQ_PARMS_OP, irq_dio_buff, 8);
}

void set_tx_mode(uint32_t timeout) {
	uint8_t timeout_buff[3];
    timeout_buff[0] = (uint8_t) (timeout >> 16) & 0xFF;
    timeout_buff[1] = (uint8_t) (timeout >> 8) & 0xFF;
    timeout_buff[2] = (uint8_t) timeout & 0xFF;

	sendCommand(spi_handle, SET_TX_MODE_OP, timeout_buff, 3);
}

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

	uint8_t status;
	uint8_t cmd_addr[3] = {READ_REG_OP, (uint8_t) ((reg_addr >> 8) & (0x00FF)),
						  (uint8_t) (reg_addr &0x00FF)};
	
	int spiWrite = lgSpiWrite(spi_handle, (const char *) cmd_addr, 3); 
	int spiRead_Write = lgSpiXfer(spi_handle, (const char *) NO_OPERATION, &status, 1);//dif from quad, 
	int spiRead = lgSpiRead(spi_handle, (char *) data, len); //reads data into paramters

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

