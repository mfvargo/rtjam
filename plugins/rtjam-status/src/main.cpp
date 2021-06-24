#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h>
#include "StatusLight.hpp"
#include "LightData.hpp"

using namespace std;

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

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(delay));
        status.set(lightData.m_pLightSettings->status);
        inputOne.set(lightData.m_pLightSettings->inputOne);
        inputTwo.set(lightData.m_pLightSettings->inputTwo);
    };
    return 0;
}
