#include "CodecControl.hpp"



void CodecControlAndStatus::init(void)
{

    // reset the codec at startup - reset line tied to GPIO17 on CM4   
    wiringPiSetupGpio();    // setup wiring Pi - use GPIO pin names 
    pinMode(17, OUTPUT);
    digitalWrite(17, HIGH);
    delayMicroseconds(20000);
    digitalWrite(17, LOW);
  
    // Setup I2C communication
    fd = wiringPiI2CSetup(TLV320AIC3101_ADDRESS);


    // Set Reg 0 - select Page 0 
    // wiringPiI2CWriteReg8(fd, 0, 0);
    
    // test
    wiringPiI2CWriteReg8(TLV320AIC3101_ADDRESS, 0, 0);


    // Init Page 0 Registers
    for(unsigned char i = 0; i <= 20; i++)
    {
        wiringPiI2CWriteReg8(fd, m_codecRegDataP0[i][0], m_codecRegDataP0[i][1]);   
    }
    
    // Set Reg 0 - select Page 1
    wiringPiI2CWriteReg8(fd, 0, 1);   // set page to 1

    // Init Page 1 Register (ADC HPF filter coeffs - 10Hz to remove DC offset)
    for(unsigned char i = 0; i <= 14; i++)
    {
        wiringPiI2CWriteReg8(fd, m_codecRegDataP1[i][0], m_codecRegDataP1[i][1]);   
    }

    // Set Reg 0 - select Page 0
    wiringPiI2CWriteReg8(fd, 0, 0);   // set page back to 0

    // Set Reg 12 - Enable Left and Right ADC Channel HPF  
    wiringPiI2CWriteReg8(fd, 12, 0x50);

    // Set Reg 107 - set HPF to use custom coeffs loaded above
    wiringPiI2CWriteReg8(fd, 0, 0);  

}


// Pot Input Routine (called in main loop) - Read pots and send I2C commands to codec if value changed
//
// filter coeff for cuttoff freq wc = alpha
// y_n = alpha*x_n + (1-alpha)*y_n_1
// alpha = 1-exp(-wc/Fs)
//
//    float temp = 0;
//    int out = 0;    
//    temp = (float)a/(float)b;
//    out = (int)round(temp);
//
void CodecControlAndStatus::updateVolumes(void)
{
      
    // Check pot values and update gain registers in the '3101 if value changes
    ADC_ScanInputs();
    
    // Pot 1 - channel 0 - Instrument input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot1Filt;
    wiringPiI2CWriteReg8(fd, 15, (unsigned char)(m_adcValue[0]/5));
        
    // Pot 2 - channel 1 - mic/headset input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot2Filt;
    wiringPiI2CWriteReg8(fd, 16, m_adcValue[1]/4);
        
    // Pot 3 - channel 2 - Headphone amp gain]
    m_temp = ALPHA*m_adcValue[2] + (1-ALPHA)*m_pot3Filt;
     {
        temp = (255 - m_adcValue[2])/2; // invert and scale pot value 
        temp |= 0x80;    // set bit 7 (enable DAC-HP path)
        wiringPiI2CWriteReg8(fd, 47, temp); // update L and R volumes
        wiringPiI2CWriteReg8(fd, 64, temp);      
     }
        
        // store current state for next time through loop
        m_lastPot1Value = m_pot1Filt;
        m_lastPot2Value = m_pot2Filt;
        m_lastPot3Value = m_pot3Filt;    
        
}



unsigned int CodecControlAndStatus::ADC_Init(void)
{

    fd = wiringPiI2CSetup(ADS7991_ADDRESS);

}


unsigned int CodecControlAndStatus::ADC_ScanInputs(unsigned char channel)
{

    // read 3 ADC channels - sequential??? TODO - change to single?
    for(unsigned int i=0; i<3; i++)
    {
        m_adcControlReg = 0x01 << i; // set channel bit (shift left each time through)       
        wiringPiI2CWrite(fd, m_adcControlReg);


        m_adcResultHigh = wiringPiI2CRead(fd);
        m_adcResultLow = wiringPiI2CRead(fd);
        
        // save and mask off channel information, then convert adc result to 16 bits
        m_adcChannel = (m_adcResultHigh & 0x30) >> 4;
        m_temp = (adcResultHigh & 0x0f);
        m_temp <<= 8;
        m_adcValue[m_channel] = m_adcResultHigh | m_adcResultLow;
        delayMicroseconds(2); 

    }


}

