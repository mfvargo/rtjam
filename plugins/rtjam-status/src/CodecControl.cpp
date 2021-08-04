#include <iostream>
//#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

#include "CodecControl.hpp"
#include "GpioPin.hpp"

#include <thread>

using namespace std;

int CodecControlAndStatus::init(void)
{
    // Open I2C1 bus for read/write
    if ((m_file = open(m_filename, O_RDWR)) < 0)
    {
        cerr << "Failed to open i2c bus, no hardware there?" << endl;
        return (-1);
    }

    // initialize filters for pot inputs (sample rate is poll rate)
    m_pot1Filter.init(10.0f, 100.0f);
    m_pot2Filter.init(10.0f, 100.0f);
    m_pot3Filter.init(10.0f, 100.0f);

    // initialize GPIO 17 as output - RSTN line to codec (reset = low, active = high)
    GpioPin resetPin;
    resetPin.init(17, "resetPin");
    // reset the codec at startup - reset line tied to GPIO17 on CM4
    resetPin.set(false);
    // hold it low for 200msec
    std::this_thread::sleep_for(std::chrono::microseconds(200000));
    resetPin.set(true);
    // delay before sending I2C commands
    std::this_thread::sleep_for(std::chrono::microseconds(200000));

    // Initialize the Codec I2C registers
    // Mode = slave, LRCLK = 48kHz, SCLK = 64xFs

    // setup I2C to write to TLC320AIC3101 codec at address 0x18
    if (ioctl(m_file, I2C_SLAVE, 0x18) < 0)
    {
        cerr << "Failed to access codec on i2c bus" << endl;
        return (-1);
    }

    // Set Reg 0 - select Page 0
    m_I2cDataBuffer[0] = 0;
    m_I2cDataBuffer[1] = 0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
        cerr << "Failed to write to i2c bus" << endl;
        return (-1);
    }

    // Init Page 0 Registers
    for (unsigned char i = 0; i <= 20; i++)
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
        return (-1);
    }

    // Init ADC HPF filter coeffs - 10Hz filter to remove DC offset
    for (unsigned char i = 0; i <= 14; i++)
    {
        m_I2cDataBuffer[0] = m_codecRegDataP1[i][0];
        m_I2cDataBuffer[1] = m_codecRegDataP1[i][1];
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to write to i2c bus" << endl;
            return (-1);
        }
    }

    // Set Reg 0 - select Page 0
    m_I2cDataBuffer[0] = 0;
    m_I2cDataBuffer[1] = 0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
        cerr << "Failed to write to i2c bus" << endl;
        return (-1);
    }

    // Set Reg 12 - Enable Left and Right ADC Channel HPF
    //wiringPiI2CWriteReg8(m_codecI2cAddress, 12, 0x50);
    m_I2cDataBuffer[0] = 12;
    m_I2cDataBuffer[1] = 0X50;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
        cerr << "Failed to write to i2c bus" << endl;
        return (-1);
    }

    // Set Reg 107 - set HPF to use custom coeffs loaded above
    //wiringPiI2CWriteReg8(m_codecI2cAddress, 0, 0);
    m_I2cDataBuffer[0] = 107;
    m_I2cDataBuffer[1] = 0xc0;
    if (write(m_file, m_I2cDataBuffer, 2) != 2)
    {
        cerr << "Failed to write to i2c bus" << endl;
        return (-1);
    }

    return 1;
}

// Pot Input Routine (called in main loop) - Read pots and send I2C commands to codec if value changed
//

void CodecControlAndStatus::updateVolumes(void)
{

    // Check pot values and update gain registers in the '3101 if value changes
    ADC_ScanInputs();

    // filter adc values
    m_pot1Value = (int)(m_pot1Filter.getSample((float)(m_adcValue[0])));
    m_pot2Value = (int)(m_pot2Filter.getSample((float)(m_adcValue[1])));
    m_pot3Value = (int)(m_pot3Filter.getSample((float)(m_adcValue[2])));

    // test only - print pot values after filtering
    // cout << "Pot values:" << m_pot1Value << "   " << m_pot2Value << "   " << m_pot3Value << endl;

    // setup I2C bus to write to codec address 0x18
    int m_codecI2cAddr = 0x18;
    if (ioctl(m_file, I2C_SLAVE, m_codecI2cAddr) < 0)
    {
        cerr << "Failed to access i2c bus" << endl;
        exit(-1);
    }

    // Pot 1 - channel 0 - Instrument input gain
    if (abs(m_pot1Value - m_lastPot1Value) > 2)
    {
        m_I2cDataBuffer[0] = 15;
        m_I2cDataBuffer[1] = m_pot1Value / 5;
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to write instrument gain to i2c bus" << endl;
        }

        // test only - print gain value
        //  cout << "Instrument Gain =  " << (int)(m_I2cDataBuffer[1]) << endl;
    }

    // Pot 2 - channel 1 - mic/headset input gain
    if (abs(m_pot2Value - m_lastPot2Value) > 2)
    {

        m_I2cDataBuffer[0] = 16;
        m_I2cDataBuffer[1] = m_pot2Value / 4;
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to write mic gain to i2c bus" << endl;
        }

        // test only - print gain value
        //cout << "Microphone Gain =  " << (int)(m_I2cDataBuffer[1]) << endl;
    }
    // Pot 3 - channel 2 - Headphone amp gain
    if (abs(m_pot3Value - m_lastPot3Value) > 2)
    {

        int temp = (255 - m_pot3Value / 2); // invert and scale pot value
        temp |= 0x80;                       // set bit 7 (enable DAC-HP path)

        m_I2cDataBuffer[0] = 47;
        m_I2cDataBuffer[1] = temp;
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to write left headphone gain to i2c bus" << endl;
        }

        m_I2cDataBuffer[0] = 64;
        m_I2cDataBuffer[1] = temp;
        if (write(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to write right headphone gain to i2c bus" << endl;
        }

        // test only - print gain value
        //cout << "Headphone Gain =  " <<  (int)(m_I2cDataBuffer[1]) << endl;
    }

    // test only - print delta for pot 3
    // cout << "Delta = " << abs(m_pot3Value - m_lastPot3Value) << endl;

    // store current state for next time through loop
    m_lastPot1Value = m_pot1Value;
    m_lastPot2Value = m_pot2Value;
    m_lastPot3Value = m_pot3Value;

    // test only - print pot values after filtering
    //cout << "Last pot values:" << m_lastPot1Value << "   " << m_lastPot2Value << "   " << m_lastPot3Value << endl;
}

void CodecControlAndStatus::ADC_ScanInputs(void)
{
    // setup I2C to write to ADC at address 0x29
    if (ioctl(m_file, I2C_SLAVE, 0x29) < 0)
    {
        cerr << "Failed to access i2c bus" << endl;
        exit(-1);
    }

    // write to ADC to start conversion
    m_adcCommand[0] = 0b11110000; // CH3-CH0,
    if (write(m_file, m_adcCommand, 1) != 1)
    {
        cerr << "Failed to write to adc on i2c bus" << endl;
        exit(-1);
    }

    // wait for conversion before reading result back from part
    std::this_thread::sleep_for(std::chrono::microseconds(10));

    // read adc conversion result for channels 0-2 (pots 1-3)
    for (unsigned int i = 0; i < 3; i++)
    {

        // read 2 byte result back from ADC
        if (read(m_file, m_I2cDataBuffer, 2) != 2)
        {
            cerr << "Failed to read adc from i2c bus" << endl;
        }

        // save and mask off channel information, then convert adc result to 8 bits
        m_adcChannel = ((m_I2cDataBuffer[0] & 0x30) >> 4);
        m_adcValue[m_adcChannel] = ((m_I2cDataBuffer[0] & 0x0F) << 8) + m_I2cDataBuffer[1];

        // scale down to 8 bits for knobs
        m_adcValue[m_adcChannel] /= 16;

        // test only - print raw ADC values
        //cout << "ADC Channel " << m_adcChannel << "= " << m_adcValue[i] << endl;
    }
}
