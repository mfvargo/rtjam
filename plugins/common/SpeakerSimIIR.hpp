#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class SpeakerSimIIR : public Effect
{
public:
  enum CabinetType
  {
    _1x12,
    _2x12,
    _4x10,
    _4x12
  };

  void init() override
  {
    Effect::init();
    m_name = "Speaker Sim";
    EffectSetting setting;
    setting.init(
        "Level",                  // Name
        EffectSetting::floatType, // Type of setting
        -6.0,                     // Min value
        12.0,                     // Max value
        0.25,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "cabinetType",          // Name
        EffectSetting::intType, // Type of setting
        CabinetType::_1x12,     // Min value
        CabinetType::_4x12,     // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"1x12", "2x12", "4x10", "4x12"});
    setting.setIntValue(CabinetType::_4x10);
    addSetting(setting);

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
    m_level = getSettingByName("Level").getFloatValue();
    m_cabinetType = (CabinetType)getSettingByName("cabinetType").getIntValue();

    setParams();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      float value = m_cabHighpass.getSample(input[i]); // cabinet low freq roll-off
      value = m_cabNotch.getSample(value);             // notch
      value = m_cabHighshelf.getSample(value);         // high shelf (peaking in mids)
      value = m_cabLowpass.getSample(value);           // 4th order LPF - high freq rolloff
      value = m_cabLowpass.getSample(value);
      output[i] = m_level * value;
    }
  };

private:
  float m_level;

  int m_cabinetType;

  BiQuadFilter m_cabHighpass, m_cabNotch, m_cabHighshelf, m_cabLowpass;

  void setParams()
  {

    switch (m_cabinetType)
    {
    case CabinetType::_1x12:
      m_cabHighpass.init(BiQuadFilter::FilterType::HighPass, 90, 1.0, 0.707, 48000);
      m_cabNotch.init(BiQuadFilter::FilterType::Notch, 700, -16, 2, 48000);
      m_cabHighshelf.init(BiQuadFilter::FilterType::HighShelf, 800, 8, 0.707, 48000);
      m_cabLowpass.init(BiQuadFilter::FilterType::LowPass, 4000, 1.0, 0.707, 48000);
      break;

    case CabinetType::_2x12:
      m_cabHighpass.init(BiQuadFilter::FilterType::HighPass, 90, 1.0, 0.707, 48000);
      m_cabNotch.init(BiQuadFilter::FilterType::Notch, 550, -16, 2, 48000);
      m_cabHighshelf.init(BiQuadFilter::FilterType::HighShelf, 700, 8, 0.707, 48000);
      m_cabLowpass.init(BiQuadFilter::FilterType::LowPass, 4000, 1.0, 0.707, 48000);
      break;

    case CabinetType::_4x10:
      m_cabHighpass.init(BiQuadFilter::FilterType::HighPass, 70, 1.0, 0.707, 48000);
      m_cabNotch.init(BiQuadFilter::FilterType::Notch, 400, -16, 2, 48000);
      m_cabHighshelf.init(BiQuadFilter::FilterType::HighShelf, 400, 8, 0.707, 48000);
      m_cabLowpass.init(BiQuadFilter::FilterType::LowPass, 4000, 1.0, 0.707, 48000);
      break;

    case CabinetType::_4x12:
      m_cabHighpass.init(BiQuadFilter::FilterType::HighPass, 40, 1.0, 0.707, 48000);
      m_cabNotch.init(BiQuadFilter::FilterType::Notch, 300, -12, 2, 48000);
      m_cabHighshelf.init(BiQuadFilter::FilterType::HighShelf, 500, 5, 0.707, 48000);
      m_cabLowpass.init(BiQuadFilter::FilterType::LowPass, 4200, 1.0, 0.707, 48000);
      break;
    }
  };
};
