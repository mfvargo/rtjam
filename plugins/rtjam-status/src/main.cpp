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

    LightData lightData;
    StatusLight::startInit();
    StatusLight status, inputOne, inputTwo;
    status.init(StatusLight::status);
    inputOne.init(StatusLight::inputOne);
    inputTwo.init(StatusLight::inputTwo);

    lightData.m_pLightSettings->inputOne = green;
    lightData.m_pLightSettings->inputTwo = green;
    lightData.m_pLightSettings->status = green;

    int rtjamHardwareDetected = 0;

    // Codec init and gain/volume control
    CodecControlAndStatus codecControl;

    // // init codec - function inits hardware and returns 1 if custom hardware detected
    // if (codecControl.init() == 1)
    // {
    //     rtjamHardwareDetected = 1; // rtjam hardware detected
    // }
    // else
    // {
    //     rtjamHardwareDetected = 0; // standard Rpi setup
    // };

    // main status poll loop
    //  - updates LEDs
    //   reads pots, update codec gain control registers if custom board detected
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(10000));
        status.set(lightData.m_pLightSettings->status);
        inputOne.set(lightData.m_pLightSettings->inputOne);
        inputTwo.set(lightData.m_pLightSettings->inputTwo);

        // poll the hardware if custom board detected
        if (rtjamHardwareDetected == 1)
        {
            codecControl.updateVolumes();
        }
    }
    return 0;
}
