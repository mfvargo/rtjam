#pragma once

#include "DistortionModelerBase.hpp"
#include "SignalBlock.hpp"

class DistortionTubeDrive : public DistortionModelerBase
{
public:
    void init() override
    {
        // Setup base class stuff
        Effect::init();

        // What is this effects name?
        m_name = "TubeOverdrive";

        // Now setup the settings this effect can receive.
        EffectSetting setting;

        setting.init(
            "drive",                 // Name
            EffectSetting::floatType, // Type of setting
            0.0,                      // Min value
            80.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(6.0);
        addSetting(setting);

        
        setting.init(
            "treble",                 // Name
            EffectSetting::floatType, // Type of setting
            -30.0,                    // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(0.0);
        addSetting(setting);

        setting.init(
            "level",                  // Name
            EffectSetting::floatType, // Type of setting
            -60.0,                    // Min value
            12.0,                      // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(-12.0);
        addSetting(setting);

        loadFromConfig();
    };

    void loadFromConfig() override
    {
        // Read the settings from the map and apply them to our copy of the data.

        m_hpf1Freq = 190.0;
        m_lpf1Freq = 6500.0;
        m_gain1 = getSettingByName("drive").getFloatValue();
        m_clip1Type = SignalBlock::ClipType::soft;
        m_hpf2Freq = 55;
        m_lpf2Freq = 2250;
        //m_gain2 = getSettingByName("drive2").getFloatValue();
        //m_clip2Type = (SignalBlock::ClipType)getSettingByName("clip2Type").getIntValue();
        m_hpf3Freq = 110;
        m_lpf3Freq = 8700;
        m_toneBassCutBoost = 14;
        m_toneBassFreq = 150;
        m_toneMidrangeCutBoost = 18;
        m_toneMidrangeQ = 1;
        m_toneMidrangeFreq = 730;
        m_toneTrebleCutBoost = getSettingByName("treble").getFloatValue();
        m_toneTrebleFreq = 3000.0;
        m_level = getSettingByName("level").getFloatValue();
        m_dryLevel = 0;

        DistortionModelerBase::loadFromConfig();
    }
};