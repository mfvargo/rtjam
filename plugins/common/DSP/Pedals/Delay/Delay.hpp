#pragma once

#include "DelayBase.hpp"

class SigmaDelay : public DelayBase
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    DelayBase::init();
    // What are we?
    m_name = "Delay";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "duration",               // Name
        EffectSetting::floatType, // Type of setting
        2,                        // Min value
        500.0,                    // Max value
        2,                        // Step Size
        EffectSetting::msec);
    setting.setFloatValue(250.0); // 1/8 note at 120BPM = 250msec.
    addSetting(setting);

    setting.init(
        "feedback",               // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.1,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.1);
    addSetting(setting);

    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.0,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.5);
    addSetting(setting);

    setting.init(
        "drift",                  // Name
        EffectSetting::floatType, // Type of setting
        -60.0,                    // Min value
        -25,                      // Max value
        1,                        // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-42);
    addSetting(setting);

    setting.init(
        "rate",                   // Name
        EffectSetting::floatType, // Type of setting
        0.1,                      // Min value
        5.0,                      // Max value
        0.1,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(1.4);
    addSetting(setting);

    setting.init(
        "delayMode",            // Name
        EffectSetting::intType, // Type of setting
        DelayMode::digital,     // Min value
        DelayMode::highpass,    // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"Dig", "Ana", "HPF"});
    setting.setIntValue(DelayMode::digital);
    addSetting(setting);

    // Do some init stuff
    m_writePointerIndex = 0;

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    m_currentDelayTime = getSettingByName("duration").getFloatValue();
    m_feedback = getSettingByName("feedback").getFloatValue();
    m_level = getSettingByName("level").getFloatValue();
    m_drift = getSettingByName("drift").getFloatValue();
    m_rate = getSettingByName("rate").getFloatValue();
    m_delayMode = (DelayMode)getSettingByName("delayMode").getIntValue();

    // Activate the config in the base class
    DelayBase::loadFromConfig();
  }
};