#pragma once

#include <cmath>
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

class SignalBlock
{
public:
  virtual float getSample(float input) = 0;

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

  static float getFramePower(const float *input, int framesize)
  {
    // linear calcution.  sum of the squares / number of values
    float power = 0.0;
    for (int i = 0; i < framesize; i++)
      power += pow(input[i], 2);
    power /= framesize;
    // Now turn into db
    if (power > 1E-6)
      return 10 * log10(power);

    return -60.0f;
  };

  enum ClipType
  {
    hard,
    soft,
    asymmetric,
    even,
  };

  static float clipSample(ClipType type, float sampleIn)
  {
    float output = sampleIn;
    switch (type)
    {
    case hard:
      if (sampleIn > 0.5)
        output = 0.5;
      if (sampleIn < -0.5)
        output = -0.5;
      break;
    case soft:
      output = sampleIn / (1 + fabs(sampleIn));
      break;
    case asymmetric:
      if (sampleIn > 0)
        output = sampleIn / (1 + fabs(sampleIn));
      if (sampleIn < 0)
        output = sampleIn / (1 + fabs(3 * sampleIn));
      break;
    case even:
      output = fabs(sampleIn) / (1 + fabs(sampleIn));
      break;
    };
    return output;
  };
};