#pragma once

#include "EffectChain.hpp"
#include "Delay.hpp"
#include "HighPassFilter.hpp"
#include "MonoVerb.hpp"
#include "Distortion.hpp"
#include "Tremelo.hpp"
#include "ToneStack.hpp"

class PedalBoard
{
public:
  void init()
  {
    m_distortion.init();
    m_distortion.setByPass(true);
    m_delay.init();
    m_delay.setByPass(true);
    m_reverb.init();
    m_reverb.setByPass(true);
    m_tremelo.init();
    m_tremelo.setByPass(true);
    m_toneStack.init();
    m_toneStack.setByPass(true);
    m_effectChain.push(&m_distortion);
    m_effectChain.push(&m_delay);
    m_effectChain.push(&m_reverb);
    m_effectChain.push(&m_tremelo);
    m_effectChain.push(&m_toneStack);
  }
  void process(const float *input, float *output, int framesize)
  {
    m_effectChain.process(input, output, framesize);
  }
  EffectChain m_effectChain;

private:
  Distortion m_distortion;
  SigmaDelay m_delay;
  MonoVerb m_reverb;
  Tremelo m_tremelo;
  ToneStack m_toneStack;
};