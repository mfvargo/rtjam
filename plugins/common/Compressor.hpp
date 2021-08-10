#pragma once

#include <math.h>
#include "Effect.hpp"
#include "SignalBlock.hpp"
#include "PeakDetector.hpp"


class Compressor : public Effect
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
    // What are we?
    m_name = "Compressor";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "threshold",                  // Name
        EffectSetting::floatType, // Type of setting
        -80.0,                        // Min value
        20.0,                        // Max value
        .5,                      // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-20.0);
    addSetting(setting);

setting.init(
        "ratio",                   // Name
        EffectSetting::floatType, // Type of setting
        1,                        // Min value
        100,                        // Max value
        0.25,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(1);
    addSetting(setting);

setting.init(
        "level",                   // Name
        EffectSetting::floatType, // Type of setting
        -20,                        // Min value
        20,                        // Max value
        0.5,                     // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0);
    addSetting(setting);

setting.init(
        "attack",                   // Name
        EffectSetting::floatType, // Type of setting
        2,                        // Min value
        200,                        // Max value
        0.5,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(20);
    addSetting(setting);

setting.init(
        "release",                   // Name
        EffectSetting::floatType, // Type of setting
        50,                        // Min value
        1000,                        // Max value
        0.5,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(120);
    addSetting(setting);

    loadFromConfig();

  };


  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    m_compThreshold = getSettingByName("threshold").getFloatValue();
    m_compRatio = getSettingByName("ratio").getFloatValue();
    m_compLevel = getSettingByName("level").getFloatValue();

    m_compAttackTime = (getSettingByName("attack").getFloatValue())/1000; // convert ms to secs
    m_compReleaseTime = (getSettingByName("release").getFloatValue())/1000; // convert ms to secs
    
    m_compSlope = (1-(1/m_compRatio));

    m_compPeakDetector.init(m_compAttackTime, m_compReleaseTime, 48000);

  };

  
  //  TODO - add block diagram here...

  void process(const float *input, float *output, int framesize) override
  {

    // Simple Mono Reverb
    //
    float inputLevel, compGain;

    for (int sample = 0; sample < framesize; sample++)
    {
      
      // input signal goes to peak detector sidechain
      float value = m_compPeakDetector.getSample(input[sample]);

      // convert level to dB (all gain computations are in dB)
      inputLevel = 20 * log10(value);

      // set gain to 0dB 
      compGain = 0;
      // if signal is above threshold, calculate new gain 
      if(inputLevel > m_compThreshold);
      {          
          compGain = m_compSlope*(m_compThreshold - inputLevel);
      }

      // convert gain in dB to linear value
      compGain = pow(10, (compGain / 20.0) );

      // multiply incoming signal by gain computer output (dynamic) 
      // and level (make-up gain)
      output[sample] = input[sample] * compGain * m_compLevel;

    }
    
    if(i++ > 1000)
    {
      i=0;
      std::cout << "Threshold = " << m_compThreshold << "  Level = " << inputLevel << "  Gain = " << compGain << std::endl;
    }

  };

private:
    
    PeakDetector m_compPeakDetector;

    float m_compThreshold, m_compRatio, m_compSlope, m_compLevel; 
    float m_compAttackTime, m_compReleaseTime;

    int i;
};