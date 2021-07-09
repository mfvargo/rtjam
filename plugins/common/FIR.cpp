#pragma once

#include "SignalBlock.hpp"


class FirFilter : public SignalBlock
{
public:
  

  void init(float FirCoeffs[], int N)
  {
      for(unsigned int i=0; i<N; i++)
      {
        m_firCoeffs[i] = FirCoeffs[i];
      }  

  };

  json getConfig() override
  {
    json config = {
        {"FirCoeffs", m_FirCoeffs},
       
    };
    return config;
  }

  float getSample(float input) override
  {
    
    // Copy the new sample in
    tap[tapIndex++] = input;
    if (tapIndex >= N)
    {
 	    tapIndex = 0;
    }
 
// convolution sum
// convolve input function with the impulse response
//	
 
   sum = 0;
   for(i=0; i<N; i++)
   {
 	 sum += tap[tapIndex++] * FirCoeffs[N-i];
 
	if (tapIndex >= N)
    {
	  tapIndex = 0;
    }
 
    return sum;
  
  };

public:
  float m_firCoeffs[];
  

private:
  float tap[];
  unsigned int tapIndex;
  float sum;



};
