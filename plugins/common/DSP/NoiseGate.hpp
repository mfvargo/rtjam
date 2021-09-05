#pragma once

#include "Effect.hpp"
#include "AttackHoldRelease.hpp"


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
        .05,                    // Max value
        .001,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(.004); 
    addSetting(setting);

    setting.init(
        "attack",               // Name
        EffectSetting::floatType, // Type of setting
        2,                      // Min value
        100,                        // Max value
        1,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(10);
    addSetting(setting);

    setting.init(
        "hold",               // Name
        EffectSetting::floatType, // Type of setting
        20,                      // Min value
        250,                        // Max value
        1,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(40);
    addSetting(setting);

    setting.init(
        "release",                  // Name
        EffectSetting::floatType, // Type of setting
        10,                      // Min value
        450,                      // Max value
        1,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(100);
    addSetting(setting);

    loadFromConfig();

  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_noiseGateThreshold = getSettingByName("threshold").getFloatValue();
    m_noiseGateAttack = (getSettingByName("attack").getFloatValue())/1000;
    m_noiseGateHold = (getSettingByName("hold").getFloatValue())/1000;
    m_noiseGateRelease = (getSettingByName("release").getFloatValue())/1000;
 
        // init the attack/release signal block
    m_noiseGateAttackHoldRelease.init(m_noiseGateAttack, m_noiseGateHold, m_noiseGateRelease, 48000);

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

        // generate gain control waveform (fast attack, variable hold, slow release)
        value = m_noiseGateAttackHoldRelease.getSample(triggerOut);

        // apply gain to input 
        output[sample] = input[sample] * value;

    }
  };

private:
    AttackHoldRelease m_noiseGateAttackHoldRelease;
  
    float m_noiseGateThreshold; 
    float m_noiseGateAttack;
    float m_noiseGateHold;
    float m_noiseGateRelease;
    float m_envelopeSample;

      
  
};