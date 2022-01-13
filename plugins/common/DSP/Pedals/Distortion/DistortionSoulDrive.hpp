#pragma once

#include "DistortionModelerBase.hpp"
#include "SignalBlock.hpp"

class DistortionSoulDrive : public DistortionModelerBase
{
public:
    void init() override
    {
        // Setup base class stuff
        DistortionModelerBase::init();

        // What is this effects name?
        m_name = "SoulDrive";

        // Now setup the settings this effect can receive.
        EffectSetting setting;

        setting.init(
            "drive",                 // Name
            EffectSetting::floatType, // Type of setting
            0.0,                      // Min value
            60.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(9.0);
        addSetting(setting);

        
        setting.init(
            "treble",                 // Name
            EffectSetting::floatType, // Type of setting
            -10.0,                    // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(25.0);
        addSetting(setting);

        setting.init(
            "level",                  // Name
            EffectSetting::floatType, // Type of setting
            -60.0,                    // Min value
            12.0,                      // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(-4.0);
        addSetting(setting);

        loadFromConfig();
    };

    void loadFromConfig() override
    {
        // Read the settings from the map and apply them to our copy of the data.
        m_stages = 1;
        m_hpf1Freq = 120.0;
        m_lpf1Freq = 7500.0;
        m_gain1 = getSettingByName("drive").getFloatValue();
        m_clip1Type = SignalBlock::ClipType::hard;
        m_hpf2Freq = 55;
        m_lpf2Freq = 10000;
        m_hpf3Freq = 110;
        m_lpf3Freq = 8700;
        m_toneBassCutBoost = 3;
        m_toneBassFreq = 150;
        m_toneMidrangeCutBoost = 12;
        m_toneMidrangeQ = 1;
        m_toneMidrangeFreq = 630;
        m_toneTrebleCutBoost = getSettingByName("treble").getFloatValue();
        m_toneTrebleFreq = 1000.0;
        m_level = getSettingByName("level").getFloatValue();
        m_dryLevel = 0.2;

        DistortionModelerBase::loadFromConfig();
    }
};