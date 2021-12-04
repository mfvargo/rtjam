#pragma once

#include "DistortionModelerBase.hpp"

class DistortionSuperModeler : public DistortionModelerBase
{
public:
    void init() override
    {
        // Setup base class stuff
        Effect::init();
        // What is this effects name?
        m_name = "DistortionSuperModeler";

        // Now setup the settings this effect can receive.
        EffectSetting setting;

        setting.init(
            "hpf1Freq",               // Name
            EffectSetting::floatType, // Type of setting
            20,                       // Min value
            720,                      // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(20.0);
        addSetting(setting);

        setting.init(
            "lpf1Freq",               // Name
            EffectSetting::floatType, // Type of setting
            1000,                     // Min value
            15000,                    // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(20.0);
        addSetting(setting);

        setting.init(
            "drive1",                 // Name
            EffectSetting::floatType, // Type of setting
            0.0,                      // Min value
            40.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(6.0);
        addSetting(setting);

        setting.init(
            "clip1Type",                 // Name
            EffectSetting::intType,      // Type of setting
            SignalBlock::ClipType::hard, // Min value
            SignalBlock::ClipType::even, // Max value
            1,                           // Step Size
            EffectSetting::selector);
        setting.setLabels({"hard", "tube", "brit", "oct"});
        setting.setIntValue(SignalBlock::ClipType::soft);
        addSetting(setting);

        setting.init(
            "hpf2Freq",               // Name
            EffectSetting::floatType, // Type of setting
            20,                       // Min value
            720,                      // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(20.0);
        addSetting(setting);

        setting.init(
            "lpf2Freq",               // Name
            EffectSetting::floatType, // Type of setting
            1000,                     // Min value
            15000,                    // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(7000);
        addSetting(setting);

        setting.init(
            "drive2",                 // Name
            EffectSetting::floatType, // Type of setting
            0.0,                      // Min value
            25.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(6);
        addSetting(setting);

        setting.init(
            "clip2Type",                 // Name
            EffectSetting::intType,      // Type of setting
            SignalBlock::ClipType::hard, // Min value
            SignalBlock::ClipType::even, // Max value
            1,                           // Step Size
            EffectSetting::selector);
        setting.setLabels({"hard", "tube", "brit", "oct"});
        setting.setIntValue(SignalBlock::ClipType::soft);
        addSetting(setting);

        setting.init(
            "hpf3Freq",               // Name
            EffectSetting::floatType, // Type of setting
            20,                       // Min value
            720,                      // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(20.0);
        addSetting(setting);

        setting.init(
            "lpf3Freq",               // Name
            EffectSetting::floatType, // Type of setting
            1000,                     // Min value
            15000,                    // Max value
            5,                        // Step Size
            EffectSetting::linear);
        setting.setFloatValue(7000);
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
            "bassFreq",               // Name
            EffectSetting::floatType, // Type of setting
            30,                       // Min value
            300,                      // Max value
            0.5,                      // Step Size
            EffectSetting::linear);
        setting.setFloatValue(100.0);
        addSetting(setting);

        setting.init(
            "mid",                    // Name
            EffectSetting::floatType, // Type of setting
            -30.0,                    // Min value
            30.0,                     // Max value
            0.5,                      // Step Size
            EffectSetting::dB);
        setting.setFloatValue(0.0);
        addSetting(setting);

        setting.init(
            "midFreq",                // Name
            EffectSetting::floatType, // Type of setting
            300,                      // Min value
            2500,                     // Max value
            0.1,                      // Step Size
            EffectSetting::linear);
        setting.setFloatValue(700.0);
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
            "trebleFreq",             // Name
            EffectSetting::floatType, // Type of setting
            1000,                     // Min value
            7000,                     // Max value
            0.5,                      // Step Size
            EffectSetting::linear);
        setting.setFloatValue(4000.0);
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

        setting.init(
            "dryLevel",               // Name
            EffectSetting::floatType, // Type of setting
            -100.0,                   // Min value
            24,                       // Max value
            1,                        // Step Size
            EffectSetting::dB);
        setting.setFloatValue(-100.0);
        addSetting(setting);

        loadFromConfig();
    };

    void loadFromConfig() override
    {
        // Read the settings from the map and apply them to our copy of the data.

        m_hpf1Freq = getSettingByName("hpf1Freq").getFloatValue();
        m_lpf1Freq = getSettingByName("lpf1Freq").getFloatValue();
        m_gain1 = getSettingByName("drive1").getFloatValue();
        m_clip1Type = (SignalBlock::ClipType)getSettingByName("clip1Type").getIntValue();
        m_hpf2Freq = getSettingByName("hpf2Freq").getFloatValue();
        m_lpf2Freq = getSettingByName("lpf2Freq").getFloatValue();
        m_gain2 = getSettingByName("drive2").getFloatValue();
        m_clip2Type = (SignalBlock::ClipType)getSettingByName("clip2Type").getIntValue();
        m_hpf3Freq = getSettingByName("hpf3Freq").getFloatValue();
        m_lpf3Freq = getSettingByName("lpf3Freq").getFloatValue();
        m_toneBassCutBoost = getSettingByName("bass").getFloatValue();
        m_toneBassFreq = getSettingByName("bassFreq").getFloatValue();
        m_toneMidrangeCutBoost = getSettingByName("mid").getFloatValue();
        m_toneMidrangeFreq = getSettingByName("midFreq").getFloatValue();
        m_toneTrebleCutBoost = getSettingByName("treble").getFloatValue();
        m_toneTrebleFreq = getSettingByName("trebleFreq").getFloatValue();
        m_level = getSettingByName("level").getFloatValue();
        m_dryLevel = getSettingByName("dryLevel").getFloatValue();

        DistortionModelerBase::loadFromConfig();
    }
};