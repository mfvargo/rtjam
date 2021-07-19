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
    m_hpfilter.init();
    m_distortion.init();
    m_delay.init();
    m_reverb.init();
    m_tremelo.init();
    m_toneStack.init();
    // m_effectChain.push(&m_hpfilter);
    // m_effectChain.push(&m_distortion);
    // m_effectChain.push(&m_delay);
    m_effectChain.push(&m_reverb);
    // m_effectChain.push(&m_tremelo);
    // m_effectChain.push(&m_toneStack);
    m_stable = true;
  }
  void process(const float *input, float *output, int framesize)
  {
    if (m_stable)
      m_effectChain.process(input, output, framesize);
    else
      memcpy(output, input, sizeof(float) * framesize);
  }
  json getChainConfig(std::string name, int channel)
  {
    return m_effectChain.getChainConfig(name, channel);
  }
  bool setEffectSetting(json setting, int idx)
  {
    bool rval = false;
    // Check index bounds
    m_stable = false;
    if (idx < m_effectChain.size())
    {
      Effect *pEffect = m_effectChain.getEffect(idx);
      rval = pEffect->setSettingValue(setting);
    }
    m_stable = true;
    return rval;
  }

  void toggleEffect(int idx)
  {
    m_effectChain.toggleEffect(idx);
  }

private:
  bool m_stable;
  EffectChain m_effectChain;
  HighPassFilter m_hpfilter;
  Distortion m_distortion;
  SigmaDelay m_delay;
  MonoVerb m_reverb;
  Tremelo m_tremelo;
  ToneStack m_toneStack;
};