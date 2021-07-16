#pragma once

#include "EffectChain.hpp"
#include "Delay.hpp"
#include "HighPassFilter.hpp"
#include "MonoVerb.hpp"
#include "Distortion.hpp"
#include "Tremelo.hpp"
#include "ToneStack.hpp"

#include <mutex>

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
    m_effectChain.push(&m_hpfilter);
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
  json getChainConfig(std::string name, int channel)
  {
    return m_effectChain.getChainConfig(name, channel);
  }
  bool setEffectSetting(json setting, int idx)
  {
    bool rval = false;
    // Check index bounds
    if (idx < m_effectChain.size())
    {
      Effect *pEffect = m_effectChain.getEffect(idx);
      rval = pEffect->setSettingValue(setting);
    }
    return rval;
  }

  void toggleEffect(int idx)
  {
    m_effectChain.toggleEffect(idx);
  }

private:
  mutex m_mutex;
  EffectChain m_effectChain;
  HighPassFilter m_hpfilter;
  Distortion m_distortion;
  SigmaDelay m_delay;
  MonoVerb m_reverb;
  Tremelo m_tremelo;
  ToneStack m_toneStack;
};