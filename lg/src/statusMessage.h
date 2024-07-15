#include <stdio.h>
#include <stdint.h>
#ifndef spi_loopback_h
#define spi_loopback_h

#define BUSY_PIN 16
#define SX_NRESET_PIN 26

#define LOW 0
#define HIGH 1

#define READY 0
#define BUSY 1

//MARCO for delay_ms 
#define SLEEP_MS(ms) lguSleep((ms) / 1000.0)
//function protypes
int lgpio_init(void);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);
int getStatus(int spi_handle, char opcode, char *response, int response_len);
void gpio_init(int chip_handle);
void wait_on_busy(void);

void sendCommand(int spi_handle, uint8_t command, uint8_t* data, int command_len);
int send_read_command(int spi_handle, uint8_t command, uint8_t* data, int command_len);
//set function protypes

void set_standby_mode();
void set_packet_type(uint8_t packet_type);
void set_rf_frequency(uint32_t frequency_mhz);
void set_pa_config(uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_sel);
void set_tx_config(int8_t power, uint8_t ramp_time);

uint8_t read_registers(uint16_t reg_addr, uint8_t* data, uint8_t len);


uint8_t write_registers(uint16_t reg_addr, uint8_t* data, uint8_t len);

#endif 
