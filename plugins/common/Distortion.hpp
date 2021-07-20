#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

class Distortion : public Effect
{
public:
  enum ClipType
  {
    hard,
    soft,
    asymmetric,
    even,
  };

  void init() override
  {
    // Setup base class stuff
    Effect::init();
    // What is this effects name?
    m_name = "Distortion";

    // Now setup the settings this effect can receive.
    EffectSetting setting;
    setting.init(
        "drive",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        40.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(6.0);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));
    setting.init(
        "clipType",             // Name
        EffectSetting::intType, // Type of setting
        ClipType::hard,         // Min value
        ClipType::even,         // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"hard", "soft", "asym", "even"});
    setting.setIntValue(ClipType::soft);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));
    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        -30.0,                    // Min value
        10.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    // TODO: figure out how to make these programmable (tone knob?)
    m_lpfFreq = 5000;
    m_hpfFreq = 200;

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();
    std::map<std::string, EffectSetting>::iterator it;

    it = m_settingMap.find("drive");
    if (it != m_settingMap.end())
    {
      m_gain = it->second.getFloatValue();
    }

    it = m_settingMap.find("clipType");
    if (it != m_settingMap.end())
    {
      m_clipType = (ClipType)it->second.getIntValue();
    }

    it = m_settingMap.find("level");
    if (it != m_settingMap.end())
    {
      m_level = it->second.getFloatValue();
    }

    setupFilters();
  }
  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      float value = clipSample(input[i] * m_gain) * m_level;
      value = m_lpf.getSample(value);
      value = m_hpf.getSample(value);
      // value = distortionAlgorithm(value);
      output[i] = value;
    }
  };

private:
  BiQuadFilter m_hpf;        // runs at 48k on input signal
  BiQuadFilter m_lpf;        // runs at 48k on output signal
  BiQuadFilter m_upsample;   // runs at 8 * 48k after upsample
  BiQuadFilter m_downsample; // runs at 8 * 48k after downsample

  // Parameters
  float m_gain;        // gain before clip function
  float m_level;       // Overall level
  ClipType m_clipType; // what kind of clipping funciton
  float m_lpfFreq;     // frequency of the final lpf  (tone knob)
  float m_hpfFreq;     // frequency of the hpf before effect (is this a knob?)

  void setupFilters()
  {
    // Setup the biquad filter for the upsampled data
    m_hpf.init(BiQuadFilter::FilterType::HighPass, m_hpfFreq, 1.0, 1.0, 48000);
    m_lpf.init(BiQuadFilter::FilterType::LowPass, m_lpfFreq, 1.0, 1.0, 48000);
    m_upsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 48000);
    m_downsample.init(BiQuadFilter::FilterType::HighPass, 48000, 1.0, 1.0, 8 * 48000);
  };

  float distortionAlgorithm(float inputSample)
  {
    int i = 0;
    float filterOut[8];
    float upsampleBuffer[8];
    float clipOut[8];
    /* upsample to a vector 8x the size (zero padded) */
    for (i = 0; i < 8; i++)
    {
      int j = 0;
      upsampleBuffer[j] = inputSample;
      for (int k = 1; k < 7; k++)
      {
        upsampleBuffer[k] = 0;
      }
    }
    // TODO: All three of these for loops could be combined into one...

    /* filter the vector */
    for (i = 0; i < 8; i++)
    {
      // filter
      filterOut[i] = m_upsample.getSample(upsampleBuffer[i]);
    }
    // add gain to signal and clip
    // using either hard, soft, asymmetric or symmetric clipper
    for (i = 0; i < 8; i++)
    {
      filterOut[i] = 8 * m_gain * filterOut[i];
      clipOut[i] = clipSample(filterOut[i]);
    }
    // filter again
    for (i = 0; i < 8; i++)
    {
      clipOut[i] = m_downsample.getSample(clipOut[i]);
    }
    // down-sample the vector to back native sample rate
    return clipOut[0];
  };

  float clipSample(float input)
  {
    switch (m_clipType)
    {
    case hard:
      return hardClipSample(input);
      break;
    case soft:
      return softClipSample(input);
      break;
    case asymmetric:
      return asymmetricClipSample(input);
      break;
    case even:
      return evenClipSample(input);
      break;
    };
    return 0.0;
  }

  float hardClipSample(float sampleIn)
  {
    if (sampleIn > 0.5)
      return 0.5;
    if (sampleIn < -0.5)
      return -0.5;
    return sampleIn;
  };

  float softClipSample(float sampleIn)
  {
    return (sampleIn / (1 + fabs(sampleIn)));
  };

  float evenClipSample(float sampleIn)
  {
    return (fabs(sampleIn) / (1 + fabs(sampleIn)));
  };

  float asymmetricClipSample(float sampleIn)
  {
    float sampleOut;
    if (sampleIn > 0)
      sampleOut = sampleIn / (1 + fabs(sampleIn));
    else if (sampleIn < 0)
      sampleOut = sampleIn / (1 + fabs(3 * sampleIn));
    return sampleOut;
  };
};
