#pragma once

#include "EnvelopeBase.hpp"

class BassEnvelope : public EnvelopeFilterBase
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    EnvelopeFilterBase::init();
    // What are we?
    m_name = "Bass Envelope";

    // What settings can we receive?
    EffectSetting setting;
    setting.init(
        "type",                 // Name
        EffectSetting::intType, // Type of setting
        FilterType::LPF,        // Min value
        FilterType::HPF,        // Max value
        1,                      // Step Size
        EffectSetting::selector);
    setting.setLabels({"LPF", "BPF", "HPF"});
    setting.setIntValue(FilterType::BPF);
    addSetting(setting);

    setting.init(
        "frequency",              // Name
        EffectSetting::floatType, // Type of setting
        1,                        // Min value
        2000,                     // Max value
        5,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(20);
    addSetting(setting);
    setting.init(
        "resonance",              // Name
        EffectSetting::floatType, // Type of setting
        1,                        // Min value
        20,                       // Max value
        .05,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(4);
    addSetting(setting);
    setting.init(
        "sensitivity",            // Name
        EffectSetting::floatType, // Type of setting
        0,                        // Min value
        100,                      // Max value
        1,                        // Step Size
        EffectSetting::linear);
    setting.setFloatValue(50);
    addSetting(setting);
    setting.init(
        "attack",                 // Name
        EffectSetting::floatType, // Type of setting
        2,                        // Min value
        100,                      // Max value
        .5,                       // Step Size
        EffectSetting::msec);
    setting.setFloatValue(20);
    addSetting(setting);
    setting.init(
        "release",                // Name
        EffectSetting::floatType, // Type of setting
        10,                       // Min value
        1000,                     // Max value
        5,                        // Step Size
        EffectSetting::msec);
    setting.setFloatValue(250);
    addSetting(setting);
    setting.init(
        "filter level",           // Name
        EffectSetting::floatType, // Type of setting
        -10,                      // Min value
        10,                       // Max value
        .1,                       // Step Size
        EffectSetting::dB);
    setting.setFloatValue(0);
    addSetting(setting);
    addSetting(setting);
    setting.init(
        "dry level",              // Name
        EffectSetting::floatType, // Type of setting
        -80,                      // Min value
        0,                        // Max value
        .1,                       // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-80);
    addSetting(setting);

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map
    m_filterType = (getSettingByName("type").getFloatValue());
    m_envFreq = (getSettingByName("frequency").getFloatValue());
    m_envResonance = (getSettingByName("resonance").getFloatValue());
    m_envSensitivity = getSettingByName("sensitivity").getFloatValue();
    m_envAttack = (getSettingByName("attack").getFloatValue());
    m_envRelease = (getSettingByName("release").getFloatValue());
    m_filterLevel = (getSettingByName("filter level").getFloatValue());
    m_dryLevel = (getSettingByName("dry level").getFloatValue());

    EnvelopeFilterBase::loadFromConfig();
  };
};