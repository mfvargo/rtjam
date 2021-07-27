#pragma once

#include <math.h>
#include "SignalBlock.hpp"

using namespace std;

// attack/realease block for use in noise-gate
//
// implemented from MATLAB model/block
// see https://www.mathworks.com/help/audio/ref/noisegate.html
//

class AttackRelease : public SignalBlock
{

public:

  void init(float attackTime, float releaseTime, int sampleRate)
  {
 
    // calculate coeff for attack and release time (in secs)  
    m_attackCoeff = exp(-1*log(9) /  (sampleRate * attackTime));
    m_releaseCoeff = exp(-1*log(9) / (sampleRate * releaseTime));

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
         m_attackReleaseOut = m_attackCoeff * m_lastTrigger + (1 - m_attackCoeff) * triggerIn;
        
     }
     else
     {
         m_attackReleaseOut = m_releaseCoeff * m_lastTrigger + (1 - m_releaseCoeff) * triggerIn;

     }
    
     m_lastTrigger = triggerIn; // trigger(n-1) - trigger(n)


    return m_attackReleaseOut;
  

  };

private:
  bool m_lastTrigger;
  float m_attackCoeff, m_releaseCoeff;
  float m_attackReleaseOut;

};
