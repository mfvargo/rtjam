#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"

#include <iostream>
class Tremelo : public Effect
{
public:
  void init() override
  {
    // Setup base class stuff
    Effect::init();
    // What is this effects name?
    m_name = "Tremelo";

    // Now setup the settings this effect can receive.
    EffectSetting setting;
    setting.init(
        "depth",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.0,                      // Max value
        0.1,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.4);
    addSetting(setting);
    setting.init(
        "rate",                   // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        8.0,                      // Max value
        0.1,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(1.2);
    addSetting(setting);
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();
    std::map<std::string, EffectSetting>::iterator it;

    it = m_settingMap.find("depth");
    if (it != m_settingMap.end())
    {
      m_depth = it->second.getFloatValue();
    }

    it = m_settingMap.find("rate");
    if (it != m_settingMap.end())
    {
      m_rate = it->second.getFloatValue();
    }
    setParams();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = (1 + m_osc.getSample(1.0)) * input[i];
    }
  };

private:
  LowFreqOsc m_osc;
  float m_depth, m_rate;

  void setParams()
  {
    m_osc.init(LowFreqOsc::WaveShape::sineWave, m_rate, m_depth, 48000);
  };
};
