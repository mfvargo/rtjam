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
    m_attackCoeff = 27*(1 - exp(-1*(1/(6.28*attackTime*sampleRate))));
    m_releaseCoeff = 27*(1 - exp(-1*(1/(6.28*releaseTime*sampleRate))));

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
         m_attackReleaseOut = m_attackCoeff * triggerIn + (1 - m_attackCoeff) * m_lastOut;
        
     }
     else
     {
         m_attackReleaseOut = m_releaseCoeff * triggerIn + (1 - m_releaseCoeff) * m_lastOut;

     }
    
     m_lastOut = m_attackReleaseOut; // last output state


    return m_attackReleaseOut;
  

  };

private:
  bool m_lastOut;
  float m_attackCoeff, m_releaseCoeff;
  float m_attackReleaseOut;

};
