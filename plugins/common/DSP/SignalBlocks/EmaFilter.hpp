#pragma once

#include "SignalBlock.hpp"

class EmaFilter : public SignalBlock
{
public:
  

  void init(float cutoffFreq, float sampleRate)
  {

      // coeeficent alpha = 1-exp(-wc/Fs)
      m_alpha = 1 - exp((-1*cutoffFreq/2*M_PI)/sampleRate);
  
  };


  float getSample(float input) override
  {
    if(input == input)  // check for NaN
    {
      // recursive exponential moving average routing
      // out = alpha*x_n + (1-alpha)*y_n_1,
      m_EmaOut = m_alpha*input + (1 - m_alpha) * m_lastOutputSample;
      m_lastOutputSample = m_EmaOut;
    }
    return m_EmaOut;

  };

public:
  float m_alpha;
  

private:
    float m_EmaOut = 0;
    float m_lastOutputSample = 0;


};
