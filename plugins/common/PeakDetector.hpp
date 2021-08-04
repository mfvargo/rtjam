#pragma once

#include <math.h>
#include "SignalBlock.hpp"

using namespace std;

class PeakDetector : public SignalBlock
{
public:
  void init(float attackTime, float decayTime, int sampleRate)
  {

    // calculate coeff for attack and release time (in secs)
    m_attackCoeff = exp(-1 / (sampleRate * attackTime));
    m_decayCoeff = exp(-1 / (sampleRate * decayTime));
  };

  float getSample(float input) override
  {

    // peak or envelope detector using two leaky integrators:
    // for seperate attack and release times

    input = fabs(input);
    if (m_peakDetector < input)
    {
      m_peakDetector = input + m_attackCoeff * (m_peakDetector - input);
    }
    else
    {
      m_peakDetector = input + m_decayCoeff * (m_peakDetector - input);
    }

    return m_peakDetector;
  };

private:
  float m_attackCoeff, m_decayCoeff;
  float m_peakDetector;
};
