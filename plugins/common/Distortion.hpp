#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"
#include "SignalBlock.hpp"

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
        "tone",                   // Name
        EffectSetting::floatType, // Type of setting
        -1.0,                     // Min value
        1.0,                      // Max value
        0.05,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        -60.0,                    // Min value
        6.0,                      // Max value
        0.5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0.0);
    addSetting(setting);

    setting.init(
        "hpfMode",              // Name
        EffectSetting::intType, // Type of setting
        HpfMode::low,           // Min value
        HpfMode::high,          // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"Low", "Mid"}); // better names - low/mid???
    setting.setIntValue(HpfMode::low);
    addSetting(setting);

    // pre-and post-distortion filters (fixed)
    m_lpf1Freq = 5000;
    m_lpf2Freq = 5000;

    // tone filters used in crossFade tone control
    // fixed but cross-fading between the two changes tone between
    // "warm" and "bright"
    m_toneLpfFreq = 2000;
    m_toneHpfFreq = 3000;

    m_dryLevel = 0.01; // fixed for now - need to tune - add to param set?

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_drive = getSettingByName("drive").getFloatValue();
    m_clipType = (ClipType)getSettingByName("clipType").getIntValue();
    m_tone = getSettingByName("tone").getFloatValue();
    m_level = getSettingByName("level").getFloatValue();
    m_hpfMode = getSettingByName("hpfMode").getIntValue();

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
      value = clipSample(value * m_drive); // clip signal
      value = m_lpf1.getSample(value);     // filter out higher-order harmonics
      value = m_lpf1.getSample(value);     // filter out higher-order harmonics

      // Stage 2 - diode clipper
      value = clipSample(value * m_drive);
      value = m_lpf2.getSample(value); // filter out higher-order harmonics
      value = m_lpf2.getSample(value); // filter out higher-order harmonics

      // Stage 3 - Tone control
      // simple tone control that cross-fades between low and high-pass filters
      //
      value = SignalBlock::crossFade(m_toneHpf.getSample(value), m_toneLpf.getSample(value), m_tone);

      value = value + m_dryLevel * input[i]; // mix some dry signal in to add "detail"

      // value = distortionAlgorithm(value);
      output[i] = value * m_level;
    }
  };

private:
  BiQuadFilter m_hpf; // runs at 48k on input signal

  BiQuadFilter m_lpf1; // runs at 48k on clip1 output signal
  BiQuadFilter m_lpf2; // runs at 48k on clip1 output signal

  BiQuadFilter m_upsample;   // runs at 8 * 48k after upsample
  BiQuadFilter m_downsample; // runs at 8 * 48k after clip and before downsample

  BiQuadFilter m_toneLpf;
  BiQuadFilter m_toneHpf;

  // Parameters
  float m_drive;       // gain before clip functions
  float m_tone;        // tone control
  float m_level;       // Overall level
  ClipType m_clipType; // what kind of clipping funciton
  float m_lpf1Freq;    // frequency of the first clip block lpf (fixed filter)
  float m_lpf2Freq;    // frequency of the first clip block lpf (fixed filter)

  int m_hpfMode; // high-pass filter mode - low or mid

  float m_toneLpfFreq; // LPF cut-off frequency for tone control
  float m_toneHpfFreq; // HPF cut-off frequency for tone control

  float m_dryLevel; // amount of dry to add in at end of chain
                    // (to model Klon type drives or add detail to high gain model)

  void setupFilters()
  {
    // Setup the biquad filter for the upsampled data
    switch (m_hpfMode)
    {
    case HpfMode::high:
      m_hpf.init(BiQuadFilter::FilterType::HighPass, 700, 1.0, 1.0, 48000);
      break;
    case HpfMode::low:
      m_hpf.init(BiQuadFilter::FilterType::HighPass, 150, 1.0, 1.0, 48000);
    default:
      break;
    }
    m_lpf1.init(BiQuadFilter::FilterType::LowPass, m_lpf1Freq, 1.0, 1.0, 48000);
    m_lpf2.init(BiQuadFilter::FilterType::LowPass, m_lpf2Freq, 1.0, 1.0, 48000);
    m_upsample.init(BiQuadFilter::FilterType::LowPass, 48000, 1.0, 1.0, 8 * 48000);
    m_downsample.init(BiQuadFilter::FilterType::HighPass, 48000, 1.0, 1.0, 8 * 48000);
    m_toneLpf.init(BiQuadFilter::FilterType::LowPass, m_toneLpfFreq, 1.0, 1.0, 48000);
    m_toneHpf.init(BiQuadFilter::FilterType::HighPass, m_toneHpfFreq, 1.0, 1.0, 48000);
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
      filterOut[i] = 8 * m_drive * filterOut[i];
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
