#pragma once

#include "SignalBlock.hpp"


class FirFilter : public SignalBlock
{
public:
  

  void init(float FirCoeffs[N], int N)
  {

      m_firCoeffs.resize(N);
      m_tap.resize(N);
   
      for(unsigned int i = 0; i < N; i++)
      {
        m_firCoeffs[i] = FirCoeffs[i];
      }  


  };



  float getSample(float input) override
  {
    
    // Copy the new sample in
    m_tap[tapIndex++] = input;
    if (m_tapIndex >= N)
    {
 	    m_tapIndex = 0;
    }
 
// convolution sum
// convolve input function with the impulse response
//	
 
   m_sum = 0;
   for(int i = 0; i < N; i++)
   {
 	   m_sum += m_tap[m_tapIndex++] * m_FirCoeffs[N-i];
 
	  if (m_tapIndex >= N)
    {
	    m_tapIndex = 0;
    }
 
   }
    
    return m_sum;
  
  };

public:
  std::vector<float> m_firCoeffs[];

private:
  std::vector<float> m_tap;
  unsigned int m_tapIndex;
  float m_sum;

};
