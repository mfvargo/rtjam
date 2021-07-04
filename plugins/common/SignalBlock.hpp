#pragma once

#include <cmath>
#include "json.hpp"

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
};