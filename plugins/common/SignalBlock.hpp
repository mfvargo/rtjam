#pragma once

#include <cmath>
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

class SignalBlock
{
public:
  virtual float getSample(float input) = 0;
  virtual json getConfig() = 0;

  void getBlock(const float *input, float *output, int framesize)
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = getSample(input[i]);
    }
  }

  static float dbToFloat(float valueInDB)
  {
    if (valueInDB < -59.5)
    {
      return 0.0f;
    }
    return std::exp((valueInDB / 72.0f) * 72.0f / 8.656170245f);
  }

  static float FloatToDb(float value)
  {
    return (20 * std::log10(value));
  }

  static float crossFade(float input1, float input2, float crossFade)
  {

    // Equal power crossfade
    // returns float values representing the volumes of the left and right channels
    // when crossFade = -1, gain1 = 0, gain2 = 1
    // when crossFade = 0, gain1 = 0.7071, gain2 = 0.7071
    // when crossFade = 1, gain1 = 1, gain2  = 0

    float gain1 = sqrt(0.5f * (1.0f + crossFade));
    float gain2 = sqrt(0.5f * (1.0f - crossFade));
    // std::cout << gain1 << std::endl;
    float value = input1 * gain1 + input2 * gain2;

    return value;
  };
};