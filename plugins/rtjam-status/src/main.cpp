#if 1 

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h>
#include "StatusLight.hpp"
#include "LightData.hpp"
#include "CodecControl.hpp"


int main(int argc, char *argv[])
{
    
    // create instance to test control code
    
    CodecControlAndStatus codecControl;
    codecControl.init();

    // set up LEDs pins and turn on all lights at startup
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
    digitalWrite(23, HIGH);
    digitalWrite(24, HIGH);
    
    // loop forever (will print out raw ADC and gain register values to console)
    while (1)
    {

        codecControl.updateVolumes();
        delayMicroseconds(1000000);
    };
    return 0;
}
#else


#include <stdio.h>
#include <fcntl.h>			// O_RDWR
#include <stdlib.h>			// exit()
#include <sys/ioctl.h>		// ioctl()
#include <linux/i2c-dev.h>	// I2C_SLAVE
#include <unistd.h>			// read()
#include <errno.h>			// errno

int main(int argc, char **argv)
{	
	int file;
	char *filename = "/dev/i2c-1";
	if ((file = open(filename, O_RDWR)) < 0)
	{
		perror("Failed to open i2c bus");
		exit(-1);
	}
	
	int addr = 0b0101001;
	if (ioctl(file, I2C_SLAVE, addr) < 0)
	{
		perror("Failed to access i2c bus");
		exit(-1);
	}
	
while(1)
{

	char cmd[1] = {0};
	cmd[0] = 0b11110100;		// D7 to D4 : CH3 to CH0
								// D3 : REF_SEL (default 0)
								// D2 : FLTR (default 1)
								// D1 : Bit trial delay (default 0)
								// D0 : Sample delay (default 0)
								
	short debug_write = 1;
	if (debug_write == 1)
	{
		if (write(file, cmd, 1) != 1)
		{
			printf("Failed to write to i2c bus\n\n");
		}
	}
	
	char buf[10] = {0};
	float data;
	char channel;
	int i;
	for (i = 0; i < 4; i++)
	{
		if (read(file, buf, 2) != 2)
		{
			printf("Failed to read from i2c bus\n\n");
		}
		else {
			channel = ((buf[0] & 0b00110000) >> 4);
			data = (float)((buf[0] & 0x0F) << 8) + buf[1];
			//10-bit range for AD7991
			//data = data / 4095;
			printf("Channel %02d Data: %04f\n", channel + 1, data);
		}
	}

}

	return 0;
}

#endif