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
    m_name = "Sigma Reverb";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "wetDry",               // Name
        EffectSetting::floatType, // Type of setting
        -1,                        // Min value
         0,                    // Max value
         1,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(-0.1); 
    addSetting(setting);

    setting.init(
        "reverb time",               // Name
        EffectSetting::floatType, // Type of setting
        0,                      // Min value
        1,                        // Max value
        0.01,                     // Step Size
        EffectSetting::float);
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
        m_magnitude = abs(input[sample]);    // calculate magnitude of incoming signal
        if(m_magnitude >= m_noiseGateThreshold)     // check if > threshold
        {
            m_thresholdOut = 1;
        }
        else
        {
            m_thresholdOut = 0;
        }

        // generate gain control waveform (fast attack, slow release)
        m_noiseGateGain = m_noiseGateAttackRelease.getSample(m_thresholdOut);
       
        // apply gain to input 
        output[sample] = input[sample] * m_noiseGateGain;

    }
  };

private:
    AttackRelease m_noiseGateAttackRelease;
  
    float m_noiseGateThreshold; 
    float m_noiseGateAttack;
    float m_noiseGateRelease;

    float m_noiseGateGain;

    float m_magnitude;
    float m_thresholdOut;
  
  
};