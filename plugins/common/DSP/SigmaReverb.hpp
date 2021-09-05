#pragma once

#include "Effect.hpp"
#include "AllpassDelay.hpp"
#include "BiQuad.hpp"

class SigmaReverb : public Effect
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
    // What are we?
    m_name = "Sigma Reverb";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        0,                        // Min value
        1,                        // Max value
        .01,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0);
    addSetting(setting);

    setting.init(
        "time",                   // Name
        EffectSetting::floatType, // Type of setting
        0,                        // Min value
        1,                        // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.01);
    addSetting(setting);

    setting.init(
        "lowFreq",                // Name
        EffectSetting::floatType, // Type of setting
        10,                       // Min value
        250,                      // Max value
        5,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(10.0);
    addSetting(setting);

    setting.init(
        "highFreq",               // Name
        EffectSetting::floatType, // Type of setting
        400,                      // Min value
        8000,                     // Max value
        5,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(8000);
    addSetting(setting);

    loadFromConfig();

    // init the attack/release signal block
    //
    m_lap1.init(229, 0.6); // ap params
    m_lap2.init(321, 0.6);
    m_lap3.init(471, 0.6);
    m_lap4.init(803, 0.6);
    m_ap1.init(1821, 0.6);
    m_ap1b.init(2565, 0.6);
    m_ap2.init(2114, 0.6);
    m_ap2b.init(1817, 0.6);
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_reverbTime = getSettingByName("time").getFloatValue();
    m_reverbLevel = getSettingByName("level").getFloatValue();
    m_lowFreqCut = getSettingByName("lowFreq").getFloatValue();
    m_highFreqCut = getSettingByName("highFreq").getFloatValue();

    m_lfFilter.init(BiQuadFilter::FilterType::HighPass, m_lowFreqCut, 1.0, 1.0, 48000);
    m_hfFilter.init(BiQuadFilter::FilterType::LowPass, m_highFreqCut, 1.0, 1.0, 48000);
  };

  //  TODO - add block diagram here...

  void process(const float *input, float *output, int framesize) override
  {

    // Simple Mono Reverb
    //
    for (int sample = 0; sample < framesize; sample++)
    {
      float reverbIn = 1.0 * input[sample]; // scale by .2 to match FV-1 impementations

      // read samples from end of each delay line
      float delay1Out = m_delay1[m_delay1Index];
      float delay2Out = m_delay2[m_delay2Index];

      // TODO - commets/mods for test only
      // process first allpass chain - lap1->lap4
      float value = m_lap1.getSample(reverbIn);
      value = m_lap2.getSample(value);
      value = m_lap3.getSample(value);
      value = m_lap4.getSample(value);

      float sum1Out = value + (delay2Out * m_reverbTime);

      // process stretched all-pass Chain 2 - AP1->AP1B
      value = m_ap1.getSample(sum1Out);
      value = m_ap1b.getSample(value);

      // write result to delay line 1
      m_delay1[m_delay1Index] = value;
      ++m_delay1Index %= 4853;

      // read from end of delay line 1
      value = delay1Out * m_reverbTime;

      // process streched all-pass Chain 3 - AP2->AP2B
      // input to Chain 3 is delay 1 output * reverb time
      value = m_ap2.getSample(value);
      value = m_ap2b.getSample(value);

      // TODO - add lpf/hpf here
      value = m_lfFilter.getSample(value);
      value = m_hfFilter.getSample(value);

      // write result to delay line 2
      m_delay2[m_delay2Index] = value;
      ++m_delay2Index %= 5888;

      // reverb output = sum of two delay taps
      value = delay1Out + delay2Out;

      // add in reverb to dry signal
      output[sample] = input[sample] + value * m_reverbLevel;
    }
  };

private:
  AllpassDelay m_lap1, m_lap2, m_lap3, m_lap4, m_ap1, m_ap1b; // allpass filter chain 1
  AllpassDelay m_ap2, m_ap2b;                                 // allpass filter chain 2

  BiQuadFilter m_lfFilter, m_hfFilter;

  float m_lowFreqCut;  // reverb low freq cutoff
  float m_highFreqCut; // reverb ring high freq cutoff (damping)

  float m_delay1[4853]; // delay line 1
  unsigned int m_delay1Index;

  float m_delay2[5888]; // delay line 2
  unsigned int m_delay2Index;

  float m_reverbTime;  // reverb time - 0-1 = 0-inf
  float m_reverbLevel; // output level (dry + reverb * 0-1)
};