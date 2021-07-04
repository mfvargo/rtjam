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
      switch (m_clipType)
      {
      case hard:
        value = hardClipSample(value);
        break;
      case soft:
        value = softClipSample(value);
        break;
      case asymetric:
        value = asymmetricClipSample(value);
        break;
      case even:
        value = evenClipSample(value);
        break;
      };
      // value = distortionAlgorithm(value);
      output[i] = m_lpf.getSample(value);
    }
  };

private:
  BiQuadFilter m_hpf;
  BiQuadFilter m_lpf;
  BiQuadFilter m_upsample;
  BiQuadFilter m_downsample;
  float m_gain;
  ClipType m_clipType;
  float m_lpfFreq;
  float m_hpfFreq;

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
      // TODO add selector button for this....
      switch (m_clipType)
      {
      case hard:
        clipOut[i] = hardClipSample(filterOut[i]);
        break;
      case soft:
        clipOut[i] = softClipSample(filterOut[i]);
        break;
      case asymetric:
        clipOut[i] = asymmetricClipSample(filterOut[i]);
        break;
      case even:
        clipOut[i] = evenClipSample(filterOut[i]);
        break;
      };
    }
    // filter again
    for (i = 0; i < 8; i++)
    {
      clipOut[i] = m_downsample.getSample(clipOut[i]);
    }
    // down-sample the vector to back native sample rate
    return clipOut[0];
  };

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
