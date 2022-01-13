#pragma once

#include "Effect.hpp"

#include <iostream>
class Boost : public Effect
{
public:
  void init() override
  {
    Effect::init();
    m_name = "Boost";
    EffectSetting setting;
    setting.init(
        "Gain",                 // Name
        EffectSetting::floatType, // Type of setting
        0,                    // Min value
        12.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);
    
    loadFromConfig();
  };

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
    m_boost = getSettingByName("Gain").getFloatValue();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = m_boost * input[i];
    }
  };

private:
  float m_boost;
  
};
