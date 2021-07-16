#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class ToneStack : public Effect
{
public:
  void init() override
  {
    Effect::init();
    m_name = "Tone Stack";
    EffectSetting setting;
    setting.init(
        "treble",                 // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));
    setting.init(
        "mid",                    // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));
    setting.init(
        "low",                    // Name
        EffectSetting::floatType, // Type of setting
        -10.0,                    // Min value
        10.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
    std::map<std::string, EffectSetting>::iterator it;

    it = m_settingMap.find("treble");
    if (it != m_settingMap.end())
    {
      m_treble = it->second.getFloatValue();
    }
    it = m_settingMap.find("mid");
    if (it != m_settingMap.end())
    {
      m_mid = it->second.getFloatValue();
    }
    it = m_settingMap.find("low");
    if (it != m_settingMap.end())
    {
      m_bass = it->second.getFloatValue();
    }
    setParams();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = m_bass * m_fBass.getSample(input[i]);
      output[i] += m_mid * m_fMid.getSample(input[i]);
      output[i] += m_treble * m_fHigh.getSample(input[i]);
    }
  };

private:
  float m_treble, m_mid, m_bass;
  BiQuadFilter m_fBass, m_fMid, m_fHigh;

  void setParams()
  {
    // TODO: figure out how to chain some BiQuads to do this.
    m_fBass.init(BiQuadFilter::FilterType::LowShelf, 200, 1.0, 1.0, 48000);
    m_fMid.init(BiQuadFilter::FilterType::Peaking, 700, 1.0, 2.0, 48000);
    m_fHigh.init(BiQuadFilter::FilterType::HighShelf, 2000, 1.0, 1.0, 48000);
  };
};
