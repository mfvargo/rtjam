#pragma once

#include "DelayBase.hpp"

class Chorus : public DelayBase
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    DelayBase::init();
    // What are we?
    m_name = "Chorus";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "duration",               // Name
        EffectSetting::floatType, // Type of setting
        2,                        // Min value
        10.0,                     // Max value
        0.1,                      // Step Size
        EffectSetting::msec);
    setting.setFloatValue(5.0);
    addSetting(setting);

    setting.init(
        "depth",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        0.9,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.1);
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

    // Do some init stuff
    m_writePointerIndex = 0;

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    m_currentDelayTime = getSettingByName("duration").getFloatValue();
    float depth = getSettingByName("depth").getFloatValue();
    m_feedback = depth;
    m_level = depth;
    m_drift = SignalBlock::dbToFloat((depth * 45.0) - 60);
    m_rate = getSettingByName("rate").getFloatValue();

    // Activate the config in the base class
    DelayBase::loadFromConfig();
  }
};