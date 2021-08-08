#pragma once

#include <math.h>
#include "SignalBlock.hpp"

using namespace std;

// attack/realease block for use in noise-gate
//
// implemented from MATLAB model/block
// see https://www.mathworks.com/help/audio/ref/noisegate.html
//

class AttackHoldRelease : public SignalBlock
{

public:

  void init(float attackTime, float holdTime, float releaseTime, int sampleRate)
  {
 
    // calculate coeff for attack and release time (in secs)  
    m_attackCoeff = 27*(1 - exp(-1*(1/(6.28*attackTime*sampleRate))));
    m_releaseCoeff = 27*(1 - exp(-1*(1/(6.28*releaseTime*sampleRate))));
   
    // calculate number of samples to hold based on hold time in secs
    maxHoldTimeCount = (int)(48000*holdTime);

  };
  
  float getSample(float triggerIn) override
  {

    // attack/release generator 
    // 
    // trigger = 1 = attack
    // trigger = 2 = release
    // 
     if(triggerIn == 1)
     {
         m_attackHoldReleaseOut = m_attackCoeff * triggerIn + (1 - m_attackCoeff) * m_lastOut;
         holdTimeCount = 0; // reset hold time
     }
     else
     {
         if(holdTimeCount++ >= maxHoldTimeCount)  // 20ms hold starts when input goes to 0 - inc and test
         {  
            // release if hold time expired     
            holdTimeCount = maxHoldTimeCount; // hold count reset when re-triggered
            m_attackHoldReleaseOut = m_releaseCoeff * triggerIn + (1 - m_releaseCoeff) * m_lastOut;
         }

     }
    
     m_lastOut = m_attackHoldReleaseOut; // trigger(n-1) = trigger(n)


    return m_attackHoldReleaseOut;
  

  };

private:
  float m_attackCoeff, m_releaseCoeff, m_holdTime;
  float m_attackHoldReleaseOut;

  float m_lastOut;  // last trigger state
  int holdTimeCount;   // hold time, in samples
  int maxHoldTimeCount; //


};
