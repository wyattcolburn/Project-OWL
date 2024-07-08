#ifndef spi_loopback_h
#define spi_loopback_h

#define BUSY_PIN 16
#define SX_NRESET_PIN 26

#define LOW 0
#define HIGH 1

//function protypes
int lgpio_init(void);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);
#endif 
