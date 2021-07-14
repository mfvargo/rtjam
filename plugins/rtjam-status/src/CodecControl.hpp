#pragma once

#include <wiringPi.h>
#include <wiringPiI2C.h>

class CodecControlAndStatus
{

public:


// RTJamIO hardware Init and Control Functions
//
void init(void);
void updateVolumes(void);


  
private:

    // TLV320AIC3101 Codec IC register setting for RPIJamIO hardware 
    static const char s_TLV320AIC3101_ADD = 0x18;  

    // ADS7994 4 channel ADC Address
    static const char s_ADS7991_ADD = 0x28;

    // pot filter coefficient 
    float s_alpha = 0.3;

    int fd;     // I2C device address handle
    // DFM TODO int m_fd;     // I2C device address handle

    // adc read intermediate result registers
    unsigned int m_adcControlReg = 0;
    unsigned int m_adcChannel = 0;

    unsigned int m_adcResultHigh = 0;
    unsigned int m_adcResultLow = 0;

    // adc value array
    unsigned int m_adcValue[3];

    // pot filter state storage - y(n-1)
    float m_pot1Filt = 0;
    float m_pot2Filt = 0;
    float m_pot3Filt = 0;

    unsigned int m_temp = 0;


// input pot variables
    float m_pot1Value = 0;
    float m_lastPot1Value = 0;

    float m_pot2Value = 0;
    float m_lastPot2Value = 0;

    float m_pot3Value = 0;
    float m_lastPot3Value = 0;

// TLV320AIC3101 hardware specific I2C register/Setting pairs
// setting for Sigma Engineering CM4 based RtJamIO board - rev B
//
// Note:
// register is in decimal notation to match datasheet
// setting in in hex - see datasheet for bit filed definitions.
//
int m_codecRegDataP0[20][2] = { {7, 0x0A}, {8, 0xC0}, {9, 0x30}, 
{14, 0x80}, {18, 0x0F}, {19, 0x04}, {22, 0x04}, {25,0x80}, {37, 0xC0}, 
{43, 0x00}, {44, 0x00}, {47, 0x80}, {51, 0x0F}, {64, 0x80}, {65, 0x0F}, 
{82, 0x80}, {86, 0x09}, {92, 0x80}, {93, 0x09}, {101, 0x01} };

int m_codecRegDataP1[12][2] = { {65, 0x7F}, {66, 0xE9}, {67, 0x80}, 
{68, 0x17}, {69, 0x7F}, {70, 0xD4}, {71, 0x7F}, {72, 0xE9}, {73, 0x80}, 
{74, 0x17}, {75, 0x7F}, {76, 0xD4} };  


// DFM - ok here - name?
//
unsigned int ADC_Init(void);
void ADC_ScanInputs();



};
