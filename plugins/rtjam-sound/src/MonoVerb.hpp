#pragma once

#include "Effect.hpp"
#include "MVerb.h"

class MonoVerb : public Effect
{
public:
  void init() override
  {
    m_pReverb = new MVerb<float>;
    m_pReverb->setSampleRate(48000);
    m_pReverb->setParameter(MVerb<float>::DAMPINGFREQ, 0.5f);
    m_pReverb->setParameter(MVerb<float>::DENSITY, 0.5f);
    m_pReverb->setParameter(MVerb<float>::BANDWIDTHFREQ, 0.5f);
    m_pReverb->setParameter(MVerb<float>::DECAY, 0.5f);
    m_pReverb->setParameter(MVerb<float>::PREDELAY, 0.5f);
    m_pReverb->setParameter(MVerb<float>::SIZE, 0.75f);
    m_pReverb->setParameter(MVerb<float>::GAIN, 1.0f);
    m_pReverb->setParameter(MVerb<float>::MIX, 0.3f);
    m_pReverb->setParameter(MVerb<float>::EARLYMIX, 0.5f);
  };

  void setMix(float value)
  {
    m_pReverb->setParameter(MVerb<float>::MIX, value);
  }

  void process(const float *input, float *output, int frames) override
  {
    // This will do a mono reverb usins the stereo plugin...
    float left[frames];
    float right[frames];
    float *tempOut[2];
    tempOut[0] = left;
    tempOut[1] = right;
    float inLeft[frames];
    float inRight[frames];
    float *tempIn[2];
    tempIn[0] = inLeft;
    tempIn[1] = inRight;
    for (int i = 0; i < frames; i++)
    {
      inLeft[i] = input[i];
      inRight[i] = 0;
    }
    m_pReverb->process((const float **)tempIn, tempOut, static_cast<int>(frames));
    for (int i = 0; i < frames; i++)
    {
      output[i] = left[i];
    }
  };

private:
  MVerb<float> *m_pReverb;
};