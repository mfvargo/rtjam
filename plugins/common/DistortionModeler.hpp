#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"
#include "SignalBlock.hpp"

class DistortionModeler : public Effect
{
public:
  enum ClipType
  {
    hard,
    soft,
    asymmetric,
    even,
  };

  enum HpfMode
  {
    low,
    high,
  };

  void init() override
  {
    // Setup base class stuff
    Effect::init();
    // What is this effects name?
    m_name = "DistortionModeler";

    // Now setup the settings this effect can receive.
    EffectSetting setting;

    setting.init(
        "lowCut",                 // Name
        EffectSetting::floatType, // Type of setting
        20,                       // Min value
        720,                      // Max value
        5,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(20.0);
    addSetting(setting);

    setting.init(
        "drive",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        40.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(6.0);
    addSetting(setting);

    setting.init(
        "clipType",             // Name
        EffectSetting::intType, // Type of setting
        ClipType::hard,         // Min value
        ClipType::even,         // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"hard", "tube", "brit", "oct"});
    setting.setIntValue(ClipType::soft);
    addSetting(setting);

    setting.init(
        "bass",                   // Name
        EffectSetting::floatType, // Type of setting
        -15.0,                    // Min value
        15.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "bassFreq",               // Name
        EffectSetting::floatType, // Type of setting
        30,                       // Min value
        700,                      // Max value
        0.5,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(30.0);
    addSetting(setting);

    setting.init(
        "mid",                    // Name
        EffectSetting::floatType, // Type of setting
        -15.0,                    // Min value
        15.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "midFreq",                // Name
        EffectSetting::floatType, // Type of setting
        300,                      // Min value
        1500,                     // Max value
        0.1,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(300.0);
    addSetting(setting);

    setting.init(
        "treble",                 // Name
        EffectSetting::floatType, // Type of setting
        -15.0,                    // Min value
        15.0,                     // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "trebleFreq",             // Name
        EffectSetting::floatType, // Type of setting
        1000,                     // Min value
        7000,                     // Max value
        0.5,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(1000.0);
    addSetting(setting);

    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        -60.0,                    // Min value
        6.0,                      // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-6.0);
    addSetting(setting);

    setting.init(
        "dryLevel",               // Name
        EffectSetting::floatType, // Type of setting
        -100.0,                   // Min value
        0,                        // Max value
        1,                        // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-100.0);
    addSetting(setting);

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();
    std::map<std::string, EffectSetting>::iterator it;

    it = m_settingMap.find("lowCut");
    if (it != m_settingMap.end())
    {
      m_hpfFreq = it->second.getFloatValue();
    }

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

    it = m_settingMap.find("bass");
    if (it != m_settingMap.end())
    {
      m_toneBassCutBoost = it->second.getFloatValue();
    }

    it = m_settingMap.find("bassFreq");
    if (it != m_settingMap.end())
    {
      m_toneBassFreq = it->second.getFloatValue();
    }

    it = m_settingMap.find("mid");
    if (it != m_settingMap.end())
    {
      m_toneMidrangeCutBoost = it->second.getFloatValue();
    }

    it = m_settingMap.find("midFreq");
    if (it != m_settingMap.end())
    {
      m_toneMidrangeFreq = it->second.getFloatValue();
    }

    it = m_settingMap.find("treble");
    if (it != m_settingMap.end())
    {
      m_toneTrebleCutBoost = it->second.getFloatValue();
    }

    it = m_settingMap.find("trebleFreq");
    if (it != m_settingMap.end())
    {
      m_toneTrebleFreq = it->second.getFloatValue();
    }

    it = m_settingMap.find("level");
    if (it != m_settingMap.end())
    {
      m_level = it->second.getFloatValue();
    }

    it = m_settingMap.find("dryLevel");
    if (it != m_settingMap.end())
    {
      m_dryLevel = it->second.getFloatValue();
    }

    setupFilters();
  }

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      // HPF in chain - determines amount of low-freqs sent to clipper block
      // use F~140 Hz for full-range. F=720 for Tubescreamer type distorion
      float value = m_hpf.getSample(input[i]);

      // Stage 1 - first clipper (set to emulate op-amp clipping before diodes)
      value = clipSample(value * m_gain); // clip signal
      value = m_lpf1.getSample(value);    // filter out higher-order harmonics

      // Stage 2 - diode clipper
      value = clipSample(value * m_gain);
      value = m_lpf2.getSample(value); // filter out higher-order harmonics

      // Stage 3 - Tone control - 3 band EQ - low shelf, mid cut/boost, high shelf
      // Baxandall type w/ mid control
      //
      // TODO: I don't think you want to stack these.  I think you want to add the three onto the original
      // See ToneStack.hpp lines 73-75
      // COMMENT: this is ok and by design - tonestack uses parallel structure with gains on fixed filters that sum together
      // this is 3 band eq with series cascade structure and the freq and cut/boost will be varied by
      // just recalculating the filter coeffs vs a gain constant like in TS
      //
      value = m_toneBass.getSample(value);
      value = m_toneMidrange.getSample(value);
      value = m_toneTreble.getSample(value);

      // sum in some dry level for detail (to model Klon and similar pedals)
      value = value + (m_dryLevel * input[i]);

      output[i] = value * m_level;
    }
  };

private:
  BiQuadFilter m_hpf; // pre-clipping high-pass filter

  BiQuadFilter m_lpf1;       // post clipping filter 1
  BiQuadFilter m_lpf2;       // post clipping filter 1
  BiQuadFilter m_upsample;   // upsample filter - runs at 8 * 48k after upsample
  BiQuadFilter m_downsample; // downsample filter - runs at 8 * 48k after clip and before downsample

  BiQuadFilter m_toneBass;     // bass control frequency
  BiQuadFilter m_toneMidrange; // mid control frequency
  BiQuadFilter m_toneTreble;   // treble control frequency

  // Parameters
  float m_gain;            // gain before clip functions
  float m_level;           // Overall level
  ClipType m_clipType;     // what kind of clipping funciton
  float m_lpf1Freq = 5000; // frequency of the first clip block lpf (fixed filter)
  float m_lpf2Freq = 5000; // frequency of the first clip block lpf (fixed filter)
  float m_hpfFreq = 250;   // high-pass filter mode - low or mid

  float m_toneBassFreq;     // LPF cut-off frequency for tone control
  float m_toneMidrangeFreq; // HPF cut-off frequency for tone control
  float m_toneTrebleFreq;   // HPF cut-off frequency for tone control

  float m_toneBassCutBoost;     // LPF cut-off frequency for tone control
  float m_toneMidrangeCutBoost; // HPF cut-off frequency for tone control
  float m_toneTrebleCutBoost;   // HPF cut-off frequency for tone control

  float m_dryLevel; // amount of dry to add in at end of chain
                    // (to model Klon type drives or add detail to high gain model)

  void setupFilters()
  {

    // TODO - low-pass and highpass don't have cut/boost param - set to 0 (it's ignored in the coeff calcs)
    // Setup filters
    m_hpf.init(BiQuadFilter::FilterType::HighPass, m_hpfFreq, 1.0, 1.0, 48000);
    m_lpf1.init(BiQuadFilter::FilterType::LowPass, m_lpf1Freq, 1.0, 1.0, 48000);
    m_lpf2.init(BiQuadFilter::FilterType::LowPass, m_lpf2Freq, 1.0, 1.0, 48000);
    m_upsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 384000);
    m_downsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 384000);
    m_toneBass.init(BiQuadFilter::FilterType::LowShelf, m_toneBassFreq, m_toneBassCutBoost, 0.707, 48000);
    m_toneMidrange.init(BiQuadFilter::FilterType::Peaking, m_toneMidrangeFreq, m_toneMidrangeCutBoost, 0.707, 48000);
    m_toneTreble.init(BiQuadFilter::FilterType::HighShelf, m_toneTrebleFreq, m_toneTrebleCutBoost, 0.707, 48000);
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
      // filter at upsampled Fs/8 - 8th order filter
      filterOut[i] = m_upsample.getSample(upsampleBuffer[i]);
      filterOut[i] = m_upsample.getSample(upsampleBuffer[i]);
      filterOut[i] = m_upsample.getSample(upsampleBuffer[i]);
      filterOut[i] = m_upsample.getSample(upsampleBuffer[i]);
    }
    // add gain to signal and clip
    // using either hard, soft, asymmetric or symmetric clipper
    for (i = 0; i < 8; i++)
    {
      filterOut[i] = 8 * m_gain * filterOut[i];
      clipOut[i] = clipSample(filterOut[i]);
    }

    // filter at upsampled Fs/8 before downsampling - 8th order filter
    for (i = 0; i < 8; i++)
    {
      clipOut[i] = m_downsample.getSample(clipOut[i]);
      clipOut[i] = m_downsample.getSample(clipOut[i]);
      clipOut[i] = m_downsample.getSample(clipOut[i]);
      clipOut[i] = m_downsample.getSample(clipOut[i]);
    }

    // down-sample the vector back to original sample rate
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
