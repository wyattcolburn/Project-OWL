#ifndef spi_loopback_h
#define spi_loopback_h


//function protypes
void lgpio_init(void);
int spiHandle(int spiDev, int spiChannel, int spiBaud, int spiFlag);
void printBuffer(const char *buffer, int len);
#endif 
