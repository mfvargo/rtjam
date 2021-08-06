#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class BassDI : public Effect
{
public:
  void init() override
  {
    Effect::init();
    m_name = "Bass DI";
    EffectSetting setting;
    setting.init(
        "Volume",                 // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);
    setting.init(
        "Bass",                   // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);
    setting.init(
        "Mid",                    // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);
    setting.init(
        "Treble",                 // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
    m_volume = getSettingByName("Volume").getFloatValue();
    m_bass = getSettingByName("Bass").getFloatValue();
    m_mid = getSettingByName("Mid").getFloatValue();
    m_treble = getSettingByName("Treble").getFloatValue();
    setParams();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      float value = m_fBass.getSample(input[i]);
      value = m_fMid.getSample(value);
      value = m_fTreble.getSample(value);
      output[i] = m_volume * value;
    }
  };

private:
  float m_volume, m_treble, m_mid, m_bass;
  BiQuadFilter m_fBass, m_fMid, m_fTreble;

  void setParams()
  {
    m_fBass.init(BiQuadFilter::FilterType::LowShelf, 100, m_bass, 0.707, 48000);
    m_fMid.init(BiQuadFilter::FilterType::Peaking, 400, m_mid, 0.707, 48000);
    m_fTreble.init(BiQuadFilter::FilterType::HighShelf, 1000, m_treble, 0.707, 48000);
  };
};
