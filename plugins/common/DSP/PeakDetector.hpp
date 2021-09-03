#pragma once

#include <math.h>
#include "SignalBlock.hpp"

using namespace std;

class PeakDetector : public SignalBlock
{
public:
  void init(float attackTime, float releaseTime, int sampleRate)
  {

    // calculate coeff for attack and release time (in secs)
    m_attackCoeff = 27*(1 - exp(-1*(1/(6.28*attackTime*sampleRate))));
    m_releaseCoeff = 27*(1 - exp(-1*(1/(6.28*releaseTime*sampleRate))));

  };


// http://c4dm.eecs.qmul.ac.uk/audioengineering/compressors/documents/Reiss-Tutorialondynamicrangecompression.pdf



  float getSample(float input) override
  {

    // peak or envelope detector using two leaky integrators:
    // for seperate attack and release times

    input = fabs(input);
    if (m_peakDetector < input)
    {
      m_peakDetector = input * m_attackCoeff + (1 - m_attackCoeff) * m_lastOut;
    }
    else
    {
      m_peakDetector = input * m_releaseCoeff + (1- m_releaseCoeff) * m_lastOut;
    }

    m_lastOut = m_peakDetector;

    return m_peakDetector;
  };

private:
  float m_attackCoeff, m_releaseCoeff;  // attack and release coeffs
  float m_peakDetector, m_lastOut;  // peak detector output state

};
