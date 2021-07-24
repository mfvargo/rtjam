#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"

#define DELAY_BUFFER_SIZE 96000
#define LFO_GAIN -42.0

class SigmaDelay : public Effect
{
public:
  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
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
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    setting.init(
        "feedback",               // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.1,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.1);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    setting.init(
        "level",                  // Name
        EffectSetting::floatType, // Type of setting
        0.0,                      // Min value
        1.0,                      // Max value
        0.01,                     // Step Size
        EffectSetting::linear);
    setting.setFloatValue(0.5);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    setting.init(
        "drift",                  // Name
        EffectSetting::floatType, // Type of setting
        -60.0,                    // Min value
        -25,                      // Max value
        1,                        // Step Size
        EffectSetting::dB);
    setting.setFloatValue(-42);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    setting.init(
        "rate",                   // Name
        EffectSetting::floatType, // Type of setting
        0.1,                      // Min value
        5.0,                      // Max value
        0.1,                      // Step Size
        EffectSetting::linear);
    setting.setFloatValue(1.4);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));

    // Do some init stuff
    m_writePointerIndex = 0;

    loadFromConfig();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();
    std::map<std::string, EffectSetting>::iterator it;

    it = m_settingMap.find("duration");
    if (it != m_settingMap.end())
    {
      m_currentDelayTime = it->second.getFloatValue();
    }

    it = m_settingMap.find("feedback");
    if (it != m_settingMap.end())
    {
      m_feedback = it->second.getFloatValue();
    }

    it = m_settingMap.find("level");
    if (it != m_settingMap.end())
    {
      m_level = it->second.getFloatValue();
    }

    it = m_settingMap.find("drift");
    if (it != m_settingMap.end())
    {
      m_color = it->second.getFloatValue();
    }

    it = m_settingMap.find("rate");
    if (it != m_settingMap.end())
    {
      m_rate = it->second.getFloatValue();
    }

    m_osc.init(LowFreqOsc::WaveShape::sineWave, m_rate, m_color, 48000);
    m_bufferDepth = (1.0 + SignalBlock::dbToFloat(m_color)) * m_currentDelayTime * m_sampleRate; // max delay based on depth
  }

  // Simple Digital Delay Effect - Signal Flow Diagram
  //
  //          ┌───────────────────────────────────────────┐
  //          │                                           │
  //          │              ┌────────┐                   ▼
  //          │  ┌─────┐     │        │    ┌─────┐     ┌─────┐
  //  input ──┴─►│ sum ├────►│ delay  ├─┬─►│level├────►│ sum ├────►
  //             └─────┘     │        │ │  └─────┘     └─────┘
  //                ▲        └────────┘ │
  //                │                   │
  //                │        ┌────────┐ │
  //                └────────┤feedback│◄┘
  //                         └────────┘
  //
  void process(const float *input, float *output, int framesize) override
  {
    // Implement the delay
    for (int sample = 0; sample < framesize; sample++)
    {
      // pointer arithmetic for buffer wrap
      m_writePointerIndex++;
      m_writePointerIndex %= m_bufferDepth;

      // Use the low freq osc to modulate the delay
      int readIndex = m_writePointerIndex -
                      ((1 + m_osc.getSample(input[sample])) * m_currentDelayTime * m_sampleRate);

      if (readIndex < 0)
      {
        readIndex += m_bufferDepth;
      }
      // pointer arithmetic for buffer wrap
      readIndex %= m_bufferDepth;

      // return original plus delay
      output[sample] = input[sample] + m_delayBuffer[readIndex] * m_level;

      // add feedback to the buffer
      m_delayBuffer[m_writePointerIndex] = input[sample] + (m_delayBuffer[readIndex] * m_feedback);
    }
  };

private:
  LowFreqOsc m_osc;
  float m_delayBuffer[DELAY_BUFFER_SIZE]; // 1 second of delay buffer
  int m_sampleRate = 48000;
  int m_bufferDepth;
  float m_currentDelayTime = 0.0;
  int m_writePointerIndex;
  float m_feedback;
  float m_level;
  float m_color;
  float m_rate;
};