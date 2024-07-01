#include <stdio.h>
#include <pigpio.h>

/*
ls /dev/spidev* 
/dev/spidev0.0, /dev/spidev1.0
*/
int main(){
	puts("hi");
	uint8_t spiOpenVal = lgSerialOpen("/dev/spidev0.0",
									  9600,
									  0);
	if (spiOpenVal > 0)
	{
		puts("spi port open success");
	}
	else 
	{
		puts("spi port failed to open");
	}
	return(0);
}
