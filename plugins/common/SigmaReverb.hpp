#pragma once

#include "Effect.hpp"
#include "AllpassDelay.hpp"


class SigmaReverb : public Effect
{
public:

  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
    // What are we?
    m_name = "Sigma Reverb";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "reverb level",               // Name
        EffectSetting::floatType, // Type of setting
        0,                        // Min value
        1,                    // Max value
        .01,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0); 
    addSetting(setting);

    setting.init(
        "reverb time",               // Name
        EffectSetting::floatType, // Type of setting
        0,                      // Min value
        1,                        // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.01);
    addSetting(setting);

    loadFromConfig();

    // init the attack/release signal block
    //
    m_lap1.init(457, 0.6);  // ap params
    m_lap2.init(653, 0.6);  
    m_lap3.init(973, 0.6);
    m_lap4.init(1605, 0.6);
    m_ap1.init(3665, 0.6);
    m_ap1b.init(5130, 0.6);
    m_ap2.init(4228, 0.6);
    m_ap2b.init(3935, 0.6);

  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_reverbTime = getSettingByName("reverb time").getFloatValue();
    m_reverbLevel = getSettingByName("reverb level").getFloatValue();
    
  };

  //  TODO - add block diagram here...

  void process(const float *input, float *output,  int framesize) override
  {
    
    // Simple Mono Reverb 
    //
    for (int sample = 0; sample < framesize; sample++)
    {
        float reverbIn = 0.2 * input[sample];   // scale by .2 to match FV-1 impementations

        // read samples from end of each delay line
        m_delay1Out = m_delay1[m_delay1Index];
        m_delay2Out = m_delay2[m_delay2Index];

        // TODO - commets/mods for test only
        // process first allpass chain - lap1->lap4
       // float value = m_lap1.getSample(reverbIn);
       // value = m_lap2.getSample(value);
       // value = m_lap3.getSample(value);
       // value = m_lap4.getSample(value);
 
       	//float sum1Out = value + (m_delay2Out * m_reverbTime);
        float value = input[sample] + (m_delay2Out * m_reverbTime);


    	  // process stretched all-pass Chain 2 - AP1->AP1B  	  
       // value = m_ap1.getSample(sum1Out);
       // value = m_ap1b.getSample(value);

        // write result to delay line 1
        m_delay1[m_delay1Index++] = value;
        if(m_delay1Index >= 10000)  // cicular buffer 
        {                           // wrap if > len
          m_delay1Index = 0;        
        }

        // read from end of delay line 1
        value = m_delay1Out * m_reverbLevel; 

        // process streched all-pass Chain 3 - AP2->AP2B  	  
    	  // input to Chain 3 is delay 1 output * reverb time
       // value = m_ap2.getSample(value);
       // value = m_ap2b.getSample(value);

        // TODO - add lpf/hpf here 

        // write result to delay line 2
        m_delay2[m_delay2Index++] = value;
        if(m_delay2Index >= 12000)  // cicular buffer 
        {                           // wrap if > len
          m_delay2Index = 0;        
        }

        // reverb output = sum of two delay taps
        value = m_delay1[2500] + m_delay2[3900];

        // add in reverb to dry signal
        output[sample] = input[sample] + value * m_reverbLevel;

    }
  };

private:
   
    AllpassDelay m_lap1, m_lap2, m_lap3, m_lap4, m_ap1, m_ap1b;  // allpass filter chain 1
    AllpassDelay m_ap2, m_ap2b;   // allpass filter chain 2
    
    float m_delay1[10000];  // delay line 1
    unsigned int m_delay1Index;
    
    float m_delay2[12000];  // delay line 2
    unsigned int m_delay2Index;

    float m_reverbTime; // reverb time - 0-1 = 0-inf
    float m_reverbLevel;  // output level (dry + reverb * 0-1)

    float m_delay1Out,  m_delay2Out;  // temp storage for end of delay line read
  
  
};