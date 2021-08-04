
#include <iostream>
#include <thread>
#include "LightData.hpp"
#include "GpioPin.hpp"

using namespace std;

struct gpiod_chip *GpioPin::s_chip;

int main(int argc, char *argv[])
{
    bool bLight = true;
    GpioPin::InitializeGpioChip();
    GpioPin statusPin;
    statusPin.init(23, "status");
    while (true)
    {
        statusPin.set(bLight);
        cout << (bLight ? "on" : "off") << endl;
        bLight = !bLight;
        std::this_thread::sleep_for(std::chrono::microseconds(500000));
    }
}
