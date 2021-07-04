#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"

#define DELAY_BUFFER_SIZE 48000
#define LFO_GAIN -22.0

class SigmaDelay : public Effect
{
public:
  json getConfig() override
  {
    // Return the json for this block
    json config = {
        {"bypass", getByPass()},
        {"duration", m_currentDelayTime},
        {"feedback", m_feedback},
        {"level", m_level},
    };
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]);
    m_currentDelayTime = config["duration"];
    m_feedback = config["feedback"];
    m_level = config["level"];
    m_bufferDepth = (1.0 + SignalBlock::dbToFloat(LFO_GAIN)) * m_currentDelayTime * m_sampleRate / 1000;
  }

  void init() override
  {
    // Do some init stuff
    // setByPass(true);
    json config = {
        {"shape", LowFreqOsc::WaveShape::sineWave},
        {"freq", 1.40},
        {"amplitude", LFO_GAIN},
    };
    m_osc.setConfig(config);
    // m_osc.init(LowFreqOsc::WaveShape::sineWave, 1.406, -62, 48000);
    m_currentDelayTime = 500.0;                                                                          // msec
    m_bufferDepth = (1.0 + SignalBlock::dbToFloat(LFO_GAIN)) * m_currentDelayTime * m_sampleRate / 1000; // max delay based on depth
    m_writePointerIndex = 0;
    m_feedback = 0.25;
    m_level = 1.0;
  };
  void process(const float *input, float *output, int framesize) override
  {
    // See if effect is bypassed
    if (getByPass())
    {
      return byPass(input, output, framesize);
    }
    // Implement the delay
    for (int sample = 0; sample < framesize; sample++)
    {
      // output[sample] = m_osc.getSample(input[sample]);
      // pointer arithmetic for buffer wrap
      m_writePointerIndex++;
      m_writePointerIndex %= m_bufferDepth;

      // Use the low freq osc to modulate the delay
      int readIndex = m_writePointerIndex -
                      ((1 + m_osc.getSample(input[sample])) * m_currentDelayTime * m_sampleRate / 1000);

      if (readIndex < 0)
      {
        readIndex += m_bufferDepth;
      }
      // pointer arithmetic for buffer wrap
      readIndex %= m_bufferDepth;

      // return original plus delay
      output[sample] = input[sample] + m_level * m_delayBuffer[readIndex];

      // add feedback to the buffer
      m_delayBuffer[m_writePointerIndex] = input[sample] + (output[sample] * m_feedback);
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
};