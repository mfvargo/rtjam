
#include <iostream>
#include <thread>
#include "LightData.hpp"
#include "../../rtjam-status/src/StatusLight.hpp"
using namespace std;

int main(int argc, char *argv[])
{
    bool bLight = true;
    StatusLight::startInit();
    StatusLight status, inputOne, inputTwo;
    status.init(StatusLight::status);
    inputOne.init(StatusLight::inputOne);
    inputTwo.init(StatusLight::inputTwo);
    while (true)
    {
        status.set(bLight ? green : black);
        inputOne.set(bLight ? red : black);
        inputTwo.set(bLight ? orange : black);
        cout << (bLight ? "on" : "off") << endl;
        bLight = !bLight;
        std::this_thread::sleep_for(std::chrono::microseconds(500000));
    }
}
