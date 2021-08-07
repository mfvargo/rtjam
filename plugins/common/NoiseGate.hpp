#pragma once

#include "Effect.hpp"
#include "attackRelease.hpp"


class NoiseGate : public Effect
{
public:

  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
    // What are we?
    m_name = "Noise Gate";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "threshold",               // Name
        EffectSetting::floatType, // Type of setting
        .001,                        // Min value
        .1,                    // Max value
        .001,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(.001); 
    addSetting(setting);

    setting.init(
        "attack",               // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1,                        // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.01);
    addSetting(setting);

    setting.init(
        "release",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.0,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.1);
    addSetting(setting);

    loadFromConfig();

    // init the attack/release signal block
    m_noiseGateAttackRelease.init(m_noiseGateAttack, m_noiseGateRelease, 48000);

  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_noiseGateThreshold = getSettingByName("threshold").getFloatValue();
    m_noiseGateAttack = getSettingByName("attack").getFloatValue();
    m_noiseGateRelease = getSettingByName("release").getFloatValue();
 
  };

  //  TODO - add block diagram here...

  void process(const float *input, float *output,  int framesize) override
  {
    
    // Noise gate implementation
    //
    for (int sample = 0; sample < framesize; sample++)
    {
        float triggerOut;   // attack/release trigger 1=attack, 0=release   
        
        float value  = abs(input[sample]);    // calculate magnitude of incoming signal
        
        if(value >= m_noiseGateThreshold)     // check if > threshold
        {
            triggerOut = 1;
        }
        else
        {
            triggerOut = 0;
        }

        // generate gain control waveform (fast attack, slow release)
        value = m_noiseGateAttackRelease.getSample(triggerOut);
       
        // apply gain to input 
        output[sample] = input[sample] * value;

    }
  };

private:
    AttackRelease m_noiseGateAttackRelease;
  
    float m_noiseGateThreshold; 
    float m_noiseGateAttack;
    float m_noiseGateRelease;
      
  
};