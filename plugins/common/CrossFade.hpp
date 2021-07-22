#pragma once

#include <math.h>
#include "SignalBlock.hpp"


class CrossFade : public SignalBlock
{
public:
 


  void init()
  {


  }

  json getConfig() override
  {
    json config = {
        {"filterType", m_type},
        {"cutoffFreq", m_cutoffFreq},
        {"cutBoost", m_cutBoost},
        {"q", m_q},
    };
    return config;
  }

  float getSample(float input1, float input2, float crossFade)
  {
    
    // Equal power crossfade
    // returns float values representing the volumes of the left and right channels
    // when crossFade = -1, gain1 = 0, gain2 = 1
    // when crossFade = 0, gain1 = 0.7071, gain2 = 0.7071
    // when crossFade = 1, gain1 = 1, gain2  = 0

      m_gain1 = sqrt(0.5f * (1.0f + crossFade));
      m_gain2 = sqrt(0.5f * (1.0f - crossFade));

      float value = input1*m_gain1 + input2*m_gain2;

      return value;
  };

public:

float m_input1, m_input2;


private:
    float m_gain1, m_gain2;
    

};
