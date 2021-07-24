#include "CodecControl.hpp"
#include <iostream>
//#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>	
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>	
#include <unistd.h>	

using namespace std;


void CodecControlAndStatus::init(void)
{


    // setup wiring library for Pi - use GPIO pin names
    // TODO - change to libdpiod???
    wiringPiSetupGpio();    
    
    // initialize GPIO 17 as output - RSTN line to codec (reset = low, active = high)
    pinMode(17, OUTPUT);

    // reset the codec at startup - reset line tied to GPIO17 on CM4   
    digitalWrite(17, LOW);
    delayMicroseconds(200000);  // delay 200ms for codec to reset
    digitalWrite(17, HIGH);     // bring codec out of reset
    delayMicroseconds(20000);   // delay before sending I2C commands
  

    // Initialize the Codec I2C registers 
    // Mode = slave, LRCLK = 48kHz, SCLK = 64xFs
    
    // Open I2C1 bus for read/write
    if ((m_file = open(m_filename, O_RDWR)) < 0)
	{
		cerr << "Failed to open i2c bus" << endl;
		exit(-1);
	}

	// setup I2C to write to TLC320AIC3101 codec at address 0x18
	if (ioctl(m_file, I2C_SLAVE, 0x18) < 0)
	{
		cerr << "Failed to access codec on i2c bus" << endl;
		exit(-1);
	}

    // Set Reg 0 - select Page 0 
    m_I2cDataBuffer[0] = 0;
    m_I2cDataBuffer[1] = 0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write to i2c bus" << endl;
       exit(-1);
	}


    // Init Page 0 Registers
    for(unsigned char i = 0; i <= 20; i++)
    {
        m_I2cDataBuffer[0] = m_codecRegDataP0[i][0];
        m_I2cDataBuffer[1] = m_codecRegDataP0[i][1];
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
	        cerr << "Failed to write to i2c bus" << endl;
	    }
    }
    
    // Init Page 1 Registers - set Reg 0 to 1 (select Page 1)
    m_I2cDataBuffer[0] = 0;
    m_I2cDataBuffer[1] = 1;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	    cerr << "Failed to write to i2c bus" << endl;
        exit(-1);
    }

    // Init ADC HPF filter coeffs - 10Hz filter to remove DC offset
    for(unsigned char i = 0; i <= 14; i++)
    {
        m_I2cDataBuffer[0] = m_codecRegDataP1[i][0];
        m_I2cDataBuffer[1] = m_codecRegDataP1[i][1];
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
	        cerr << "Failed to write to i2c bus" << endl;
            exit(-1);
        }

    }

    // Set Reg 0 - select Page 0
    m_I2cDataBuffer[0] = 0;
    m_I2cDataBuffer[1] = 0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	    cerr << "Failed to write to i2c bus" << endl;
        exit(-1);
    }


    // Set Reg 12 - Enable Left and Right ADC Channel HPF  
    //wiringPiI2CWriteReg8(m_codecI2cAddress, 12, 0x50);
    m_I2cDataBuffer[0] = 12;
    m_I2cDataBuffer[1] = 0X50;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write to i2c bus" << endl;
       exit(-1);
	}

    // Set Reg 107 - set HPF to use custom coeffs loaded above
    //wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 0);  
    m_I2cDataBuffer[0] = 107;
    m_I2cDataBuffer[1] = 0xc0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write to i2c bus" << endl;
       exit(-1);
	}


}


// Pot Input Routine (called in main loop) - Read pots and send I2C commands to codec if value changed
//


void CodecControlAndStatus::updateVolumes(void)
{
      
    // Check pot values and update gain registers in the '3101 if value changes
    ADC_ScanInputs();
   
	// setup I2C bus to write to codec address 0x18
	int m_codecI2cAddr = 0x18; 
	if (ioctl(m_file, I2C_SLAVE, m_codecI2cAddr) < 0)
	{
		cerr << "Failed to access i2c bus" << endl;
		exit(-1);
	}

    // Pot 1 - channel 0 - Instrument input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot1Filt;
    cout << "Instrument Gain =  " << m_adcValue[0]/5 << endl;   
    m_I2cDataBuffer[0] = 15;
 	m_I2cDataBuffer[1] = m_adcValue[0]/5; 
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write instrument gain to i2c bus" << endl;
	}

    // Pot 2 - channel 1 - mic/headset input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot2Filt;
    cout << "Mic Gain =  " << m_adcValue[1]/4 << endl;
    m_I2cDataBuffer[0] = 16;
    m_I2cDataBuffer[1] = m_adcValue[1]/4;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write mic gain to i2c bus" << endl;
	}

    // Pot 3 - channel 2 - Headphone amp gain]
    m_temp = s_alpha*m_adcValue[2] + (1-s_alpha)*m_pot3Filt;
    m_temp = (255 - m_adcValue[2])/2; // invert and scale pot value 
    m_temp |= 0x80;    // set bit 7 (enable DAC-HP path)
    cout << "Headphone Gain =  " <<  m_temp << endl;

    m_I2cDataBuffer[0] = 47;
    m_I2cDataBuffer[1] = m_temp;    
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write left headphone gain to i2c bus" << endl;
	}
    
    m_I2cDataBuffer[0] = 64;
    m_I2cDataBuffer[1] = m_adcValue[1]/4;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
	   cerr << "Failed to write right headphone gain to i2c bus" << endl;
	}
            
    // store current state for next time through loop
    m_lastPot1Value = m_pot1Filt;
    m_lastPot2Value = m_pot2Filt;
    m_lastPot3Value = m_pot3Filt;    

   
}




void CodecControlAndStatus::ADC_ScanInputs(void)
{
    // setup I2C to write to ADC at address 0x29
	m_file = wiringPiI2CSetup(0x29);
//	if (ioctl(m_file, I2C_SLAVE, 0x29) < 0)
//	{
//		cerr << "Failed to access i2c bus" << endl;
//		exit(-1);
//	}
	
	// write to ADC to start conversion
	m_adcCommand[0] = 0b11110000;		// CH3-CH0, 
	wiringPiI2CWrite(m_file, 0xF0);
    if (write(m_file, m_adcCommand, 1) != 1)
    {
	   cerr << "Failed to write to adc on i2c bus" << endl;
	   exit(-1);	
    }
	
    delayMicroseconds(10);  // wait for conversion before reading result back from part

	// read adc conversion result for channels 0-2 (pots 1-3)
    for (unsigned int i = 0; i < 3; i++)
	{
        
        // read 2 bytes back from ADC
        int m_adcReadResult = wiringPiI2CReadReg16(m_file, 0);


        //if ( read(m_file, m_I2cDataBuffer, 2) != 2)
        //{
        //    cerr << "Failed to read adc from i2c bus" << endl;
        //}

        // save and mask off channel information, then convert adc result to 16 bits
        m_adcChannel = ((m_I2cDataBuffer[0] & 0x30) >> 4);
		m_adcValue[i] = (float)((m_I2cDataBuffer[0] & 0x0F) << 8) + m_I2cDataBuffer[1];
        m_adcValue[i] = m_adcReadResult;

        cout << "ADC Ch " << m_adcChannel << "= " << m_adcValue[i] << endl;
    }

#if 0 
    // read 3 ADC channels - sequential??? TODO - change to single?
    for(unsigned int i=0; i<3; i++)
    {
        m_adcControlReg = 0x01 << i + 4; // set channel bit (shift left each time through)       
    
        m_adcReadResult = wiringPiI2CReadReg16(m_adcI2cAddress, m_adcControlReg);

       // m_adcValue[i] = m_adcReadResult;

        // save and mask off channel information, then convert adc result to 16 bits
        m_adcChannel = (m_adcReadResult & 0x30) >> 12;
        m_adcValue[m_adcChannel] = m_adcReadResult/16;      // store 8 bit ADC value
        delayMicroseconds(2);
        
        std::cout << "ADC Ch " << m_adcChannel << "= " << (m_adcReadResult & 0x0fff) << endl;



    }
#endif

}

