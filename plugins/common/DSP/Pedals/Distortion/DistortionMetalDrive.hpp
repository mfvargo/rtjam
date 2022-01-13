#pragma once

#include "DistortionModelerBase.hpp"
#include "SignalBlock.hpp"

class DistortionMetalDrive : public DistortionModelerBase
{
public:
    void init() override
    {
        // Setup base class stuff
        DistortionModelerBase::init();

        // What is this effects name?
        m_name = "MetalDrive";

        // Now setup the settings this effect can receive.
        EffectSetting setting;

        setting.init(
            "drive",                 // Name
            EffectSetting::floatType, // Type of setting
            6.0,                      // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(30.0);
        addSetting(setting);

        
        setting.init(
            "bass",                   // Name
            EffectSetting::floatType, // Type of setting
            -30.0,                    // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(0.0);
        addSetting(setting);

        
        setting.init(
            "mid",                   // Name
            EffectSetting::floatType, // Type of setting
            -30.0,                    // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(0.0);
        addSetting(setting);

        
        setting.init(
            "treble",                 // Name
            EffectSetting::floatType, // Type of setting
            -20.0,                    // Min value
            10.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(0.0);
        addSetting(setting);

        setting.init(
            "level",                  // Name
            EffectSetting::floatType, // Type of setting
            -60.0,                    // Min value
            6.0,                      // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(-12.0);
        addSetting(setting);

        loadFromConfig();
    };

    void loadFromConfig() override
    {
        // Read the settings from the map and apply them to our copy of the data.
        m_stages = 2;
        m_hpf1Freq = 80.0;
        m_lpf1Freq = 3500.0;
        m_gain1 = getSettingByName("drive").getFloatValue();
        m_clip1Type = SignalBlock::ClipType::hard;
        m_hpf2Freq = 100;
        m_lpf2Freq = 8250;
        m_gain2 = getSettingByName("drive").getFloatValue();
        m_clip2Type = SignalBlock::ClipType::hard;
        m_hpf3Freq = 30;
        m_lpf3Freq = 7000;
        m_toneBassCutBoost = getSettingByName("bass").getFloatValue();
        m_toneBassFreq = 100;
        m_toneMidrangeCutBoost = getSettingByName("mid").getFloatValue();
        m_toneMidrangeQ = .1;
        m_toneMidrangeFreq = 800;
        m_toneTrebleCutBoost = getSettingByName("treble").getFloatValue();
        m_toneTrebleFreq = 2000.0;
        m_level = getSettingByName("level").getFloatValue();
        m_dryLevel = 0;

        DistortionModelerBase::loadFromConfig();
    }
};