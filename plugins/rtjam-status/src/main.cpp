#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h>
#include "StatusLight.hpp"
#include "LightData.hpp"
#include "CodecControl.hpp"


using namespace std;

#if 1
int main(int argc, char *argv[])
{
    LightData lightData;
    StatusLight::startInit();
    StatusLight status, inputOne, inputTwo;
    status.init(StatusLight::status);
    inputOne.init(StatusLight::inputOne);
    inputTwo.init(StatusLight::inputTwo);

    lightData.m_pLightSettings->inputOne = green;
    lightData.m_pLightSettings->inputTwo = green;
    lightData.m_pLightSettings->status = green;

    int i = 0;
    int delay = 10000;


    // Codec init and gain/volume control
    CodecControlAndStatus codecControl;
    codecControl.init();




    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
        status.set(lightData.m_pLightSettings->status);
        inputOne.set(lightData.m_pLightSettings->inputOne);
        inputTwo.set(lightData.m_pLightSettings->inputTwo);
    

        // TODO - test
        codecControl.updateVolumes();


    };
    return 0;
}


#else 

// Test code only

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

#endif