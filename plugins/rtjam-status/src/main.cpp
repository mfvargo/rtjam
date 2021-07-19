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
    };
    return 0;
}
