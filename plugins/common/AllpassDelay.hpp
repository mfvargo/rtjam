#pragma once

#include "SignalBlock.hpp"
#include <vector>


class AllpassDelay : public SignalBlock
{
public:
  

  void init(float delayLength, int apGain)
  {
      
      // initialize the delay line and coeffs
      m_apGain = apGain;
      m_apDelayIndex = 0;
      
      m_apDelayLength = delayLength;
      m_apDelayLine.resize(m_apDelayLength);
      
  };


  float getSample(float input) override
  {
    

        // all-pass delay or "stretched" all-pass 

        m_apDelayOut = m_apDelayLine[m_apDelayIndex]; 	// read in end of delay line  
		m_apDelayIn = input + m_apDelayOut*m_apGain;    // delay in - sum of input and fb path   
	    m_apDelayLine[m_apDelayIndex] = m_apDelayIn;	    // write to delay line - new delay sample  
		m_apDelayOut = m_apDelayOut + m_apDelayIn*(-1.0*m_apGain);	// ap out = sum of delay out and ff path  

    	if(m_apDelayIndex++ > m_apDelayLength)
    	{
    	    m_apDelayIndex = 0;
    	}


      return m_apDelayOut;
  
  };

public:
  
  

private:
    float m_apGain, m_apDelayIn, m_apDelayOut;
    
    std::vector<float> m_apDelayLine;
    int m_apDelayLength;
    int m_apDelayIndex;

};
