#include "CodecControl.hpp"
#include <iostream>
using namespace std;


void CodecControlAndStatus::init(void)
{


    // setup wiring library for Pi - use GPIO pin names
    wiringPiSetupGpio();    
    
    // Setup I2C communication to codec and ADC - device handles for wiring library
    m_codecI2cAddress = wiringPiI2CSetup(0x18);
    m_adcI2cAddress = wiringPiI2CSetup(0x29);  
    
    // initialize GPIO 17 as output - RSTN line to codec (reset = low, active = high)
    pinMode(17, OUTPUT);

    // reset the codec at startup - reset line tied to GPIO17 on CM4   
    digitalWrite(17, LOW);
    delayMicroseconds(200000);  // delay 200ms for codec to reset
    digitalWrite(17, HIGH);     // bring codec out of reset
    delayMicroseconds(20000);   // delay before sending I2C commands
  

    // Initialize the Codec I2C registers 
    // Mode = slave, LRCLK = 48kHz, SCLK = 64xFs
    
    // Set Reg 0 - select Page 0 
    wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 0);


    // Init Page 0 Registers
    for(unsigned char i = 0; i <= 20; i++)
    {
        wiringPiI2CWriteReg8(m_codecI2cAddress, m_codecRegDataP0[i][0], m_codecRegDataP0[i][1]);   
    }
    
    // Init Page 1 Registers - set Reg 0 to 1 (select Page 1)
    wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 1);   // set page to 1

    // Init ADC HPF filter coeffs - 10Hz filter to remove DC offset
    for(unsigned char i = 0; i <= 14; i++)
    {
        wiringPiI2CWriteReg8(m_codecI2cAddress, m_codecRegDataP1[i][0], m_codecRegDataP1[i][1]);   
    }

    // Set Reg 0 - select Page 0
    wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 0);   // set page back to 0

    // Set Reg 12 - Enable Left and Right ADC Channel HPF  
    wiringPiI2CWriteReg8(m_codecI2cAddress, 12, 0x50);

    // Set Reg 107 - set HPF to use custom coeffs loaded above
    wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 0);  


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
      
    m_adcI2cAddress = wiringPiI2CSetup(0x29); 

    // Check pot values and update gain registers in the '3101 if value changes
    ADC_ScanInputs();
   
   // For ADC debug only
    #if 1
        // print out adc results
        std::cout << "ADC CH0 = " << m_adcValue[0] << endl;
        std::cout << "ADC CH1 = " << m_adcValue[1] << endl;
        std::cout << "ADC CH2 = " << m_adcValue[2] << endl;
    #endif 
    
    m_adcI2cAddress = wiringPiI2CSetup(0x18); 

    // Pot 1 - channel 0 - Instrument input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot1Filt;
    std::cout << "Instrument Gain =  " << m_adcValue[0]/5 << endl;   
    wiringPiI2CWriteReg8(m_codecI2cAddress, 15, (unsigned char)(m_adcValue[0]/5));
        
    // Pot 2 - channel 1 - mic/headset input gain
    //temp = ALPHA*adcValue + (1-ALPHA)*pot2Filt;
    std::cout << "Mic Gain =  " << m_adcValue[1]/4 << endl;
    wiringPiI2CWriteReg8(m_codecI2cAddress, 16, m_adcValue[1]/4);
        
    // Pot 3 - channel 2 - Headphone amp gain]
    m_temp = s_alpha*m_adcValue[2] + (1-s_alpha)*m_pot3Filt;
    m_temp = (255 - m_adcValue[2])/2; // invert and scale pot value 
    m_temp |= 0x80;    // set bit 7 (enable DAC-HP path)
    std::cout << "Headphone Gain =  " <<  m_temp << endl;
       
    wiringPiI2CWriteReg8(m_codecI2cAddress, 47, m_temp); // update L and R volumes
    wiringPiI2CWriteReg8(m_codecI2cAddress, 64, m_temp);      
            
    // store current state for next time through loop
    m_lastPot1Value = m_pot1Filt;
    m_lastPot2Value = m_pot2Filt;
    m_lastPot3Value = m_pot3Filt;    

   

}




void CodecControlAndStatus::ADC_ScanInputs(void)
{

    // read 3 ADC channels - sequential??? TODO - change to single?
    for(unsigned int i=0; i<3; i++)
    {
        m_adcControlReg = 0x01 << i + 4; // set channel bit (shift left each time through)       
        wiringPiI2CWrite(m_adcI2cAddress, m_adcControlReg);
        std::cout << "ADC Write " << i << "= " << m_adcControlReg << endl;

        delayMicroseconds(10);  // wait for conversion before reading result back from part

        m_adcReadResult = wiringPiI2CReadReg16(m_adcI2cAddress, 0x00);
        //m_adcResultHigh = wiringPiI2CRead(m_adcI2cAddress);
        //m_adcResultLow = wiringPiI2CRead(m_adcI2cAddress);
            // For debug only


    
        std::cout << "ADC result  " << i << "= " << m_adcReadResult << endl;
        m_adcValue[i] = m_adcReadResult;

        // save and mask off channel information, then convert adc result to 16 bits
        m_adcChannel = (m_adcResultHigh & 0x30) >> 4;
        m_temp = (m_adcResultHigh & 0x0f);
        m_temp <<= 8;
        m_adcValue[m_adcChannel] = m_adcResultHigh | m_adcResultLow;
        delayMicroseconds(2);

    }


}

