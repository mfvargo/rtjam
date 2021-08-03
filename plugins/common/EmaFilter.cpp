#pragma once

#include "SignalBlock.hpp"

class EmaFilter : public SignalBlock
{
public:
  

  void init(float cutoffFreq, float sampleRate)
  {

      // coeeficent alpha = 1-exp(-wc/Fs)
      m_alpha = 1 - exp(cutoffFreq/(2*3.1415)/sampleRate)
  
  };


  float getSample(float input) override
  {
    
    // recursive exponential moving average routing
    // out = alpha*x_n + (1-alpha)*y_n_1,
    m_EmaOut = m_alpha*input + (1 - m_alpha) * m_lastOutputSample;
    m_lastOutputSample = m_EmaOut;

    return m_EmaOut;
  
  };

public:
  float m_alpha;
  

private:
    float m_EmaOut, m_lastOutputSample;


};
