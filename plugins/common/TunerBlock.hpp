#pragma once

#include "SignalBlock.hpp"

class TunerBlock : public SignalBlock
{
public:
  void init(/* What params do we need?*/)
  {
    m_detectedFrequency = 0;
  };

  float getSample(float input) override
  {
    return input;
  };

  float getFrequency()
  {
    return m_detectedFrequency;
  }

private:
  float m_detectedFrequency;
};