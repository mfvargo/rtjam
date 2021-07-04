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
    asymetric,
    even,
  };

  json getConfig() override
  {
    json config = {
        {"bypass", getByPass()},
        {"gain", m_gain},
        {"clipType", m_clipType},
        {"lowPassFreq", m_lpfFreq},
        {"hiPassFreq", m_hpfFreq},
    };
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]);
    m_gain = config["gain"];
    m_clipType = config["clipType"];
    m_lpfFreq = config["lowPassFreq"];
    m_hpfFreq = config["hiPassFreq"];
    setupFilters();
  };

  void init() override
  {
    m_lpfFreq = 150;
    m_hpfFreq = 2000;
    m_gain = 1.0;
    m_clipType = soft;
    setupFilters();
  };

  void process(const float *input, float *output, int framesize) override
  {
    if (getByPass())
    {
      return byPass(input, output, framesize);
    }
    for (int i = 0; i < framesize; i++)
    {
      float value = m_hpf.getSample(input[i]);
      value = clipSample(value);
      // value = distortionAlgorithm(value);
      output[i] = m_lpf.getSample(value);
    }
  };

private:
  BiQuadFilter m_hpf;        // runs at 48k on input signal
  BiQuadFilter m_lpf;        // runs at 48k on output signal
  BiQuadFilter m_upsample;   // runs at 8 * 48k after upsample
  BiQuadFilter m_downsample; // runs at 8 * 48k after downsample

  // Parameters
  float m_gain;        // gain before clip function
  ClipType m_clipType; // what kind of clipping funciton
  float m_lpfFreq;     // frequency of the final lpf  (tone knob)
  float m_hpfFreq;     // frequency of the hpf before effect (is this a knob?)

  void setupFilters()
  {
    // Setup the biquad filter for the upsampled data
    m_hpf.init(BiQuadFilter::FilterType::EQ_HIGH_PASS_FILTER, m_lpfFreq, 1.0, 1.0, 48000);
    m_lpf.init(BiQuadFilter::FilterType::EQ_LOW_PASS_FILTER, m_hpfFreq, 1.0, 1.0, 48000);
    m_upsample.init(BiQuadFilter::FilterType::EQ_LOW_PASS_FILTER, 48000, 1.0, 1.0, 8 * 48000);
    m_downsample.init(BiQuadFilter::FilterType::EQ_LOW_PASS_FILTER, 48000, 1.0, 1.0, 8 * 48000);
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
    case asymetric:
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
    float sampleOut;
    if (sampleIn > 0.5)
      sampleOut = 0.5;
    else if (sampleIn < 0.5)
      sampleOut = -0.5;
    return sampleOut;
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
