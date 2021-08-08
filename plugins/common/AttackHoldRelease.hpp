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
    m_attackCoeff = exp(-0.95424251 /  (sampleRate * attackTime));
    m_releaseCoeff = exp(-0.95424251 / (sampleRate * releaseTime));
    
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
         m_attackHoldReleaseOut = m_attackCoeff * lastTrigger + (1 - m_attackCoeff) * triggerIn;
         holdTimeCount = 0; // reset hold time
     }
     else
     {
         if(holdTimeCount++ >= maxHoldTimeCount)  // 20ms hold starts when input goes to 0 - inc and test
         {  
            // release if hold time expired     
            holdTimeCount = maxHoldTimeCount; // hold count reset when re-triggered
            m_attackHoldReleaseOut = m_releaseCoeff * lastTrigger + (1 - m_releaseCoeff) * triggerIn;
         }

     }
    
     lastTrigger = triggerIn; // trigger(n-1) = trigger(n)


    return m_attackHoldReleaseOut;
  

  };

private:
  float m_attackCoeff, m_releaseCoeff, m_holdTime;
  float m_attackHoldReleaseOut;

  float lastTrigger;  // last trigger state
  int holdTimeCount;   // hold time, in samples
  int maxHoldTimeCount; //


};
