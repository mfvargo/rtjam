#pragma once

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "EmaFilter.hpp"


class CodecControlAndStatus
{

public:

// RTJamIO hardware Init and Control Functions
//
int init(void);
void updateVolumes(void);


  
private:

    // TLV320AIC3101 I2C register settings - Sigma CM4 RtJamIO Rev B 
    // Note: reg # in decimal format to match datasheet setting is in 
    // hex - see datasheet for bit filed definitions.
    int m_codecRegDataP0[20][2] = { {7, 0x0A}, {8, 0xC0}, {9, 0x30}, 
    {14, 0x80}, {18, 0x0F}, {19, 0x04}, {22, 0x04}, {25,0x80}, {37, 0xC0}, 
    {43, 0x00}, {44, 0x00}, {47, 0x80}, {51, 0x0F}, {64, 0x80}, {65, 0x0F}, 
    {82, 0x80}, {86, 0x09}, {92, 0x80}, {93, 0x09}, {101, 0x01} };

    int m_codecRegDataP1[12][2] = { {65, 0x7F}, {66, 0xE9}, {67, 0x80}, 
    {68, 0x17}, {69, 0x7F}, {70, 0xD4}, {71, 0x7F}, {72, 0xE9}, {73, 0x80}, 
    {74, 0x17}, {75, 0x7F}, {76, 0xD4} };  

    // pot filter coefficient 
    float s_alpha = 0.3;

    // file handles to I2C devices 
    int m_codecI2cAddress;   // TLV320AIC3101 Codec I2C address handle
    int m_adcI2cAddress;     // AD7991 ADC I2C address handle

	int m_file;
	const char *m_filename = "/dev/i2c-1";
	char m_I2cDataBuffer[2] = {0};

    // adc read intermediate result registers
    char m_adcCommand[2] = {0};
    unsigned int m_adcChannel = 0;
    unsigned int m_adcResultHigh = 0;
    unsigned int m_adcResultLow = 0;

    // adc value array
    unsigned int m_adcValue[3];


// input pot variables
   // float m_pot1Filter = 0;
    float m_pot2Filter = 0;
    float m_pot3Filter = 0;

    EmaFilter m_pot1Filter;
 

    float m_pot1Value = 0;
    float m_lastPot1Value = 0;

    float m_pot2Value = 0;
    float m_lastPot2Value = 0;

    float m_pot3Value = 0;
    float m_lastPot3Value = 0;


    unsigned int m_temp = 0;



// private functions
//

void ADC_ScanInputs();



};
