
#include <iostream>
#include <thread>
#include "LightData.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    bool bLight = false;
    LightData lightData;
    int count = 1;
    while (count)
    {
        lightData.m_pLightSettings->status = bLight ? red : black;
        lightData.m_pLightSettings->inputOne = bLight ? orange : black;
        lightData.m_pLightSettings->inputTwo = bLight ? green : black;
        std::this_thread::sleep_for(std::chrono::microseconds(3000000));
        cout << (bLight ? "on" : "off") << endl;
        bLight = !bLight;
    }
}
