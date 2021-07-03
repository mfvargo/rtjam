#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"

class SigmaDelay : public Effect
{
public:
  json getConfig() override
  {
    // Return the json for this block
    json config = {
        {"duration", m_currentDelayTime},
        {"feedback", m_feedback},
        {"level", m_level},
    };
    return config;
  };

  void init() override
  {
    // Do some init stuff
    // setByPass(true);
    m_osc.init(LowFreqOsc::WaveShape::sineWave, 1.406, -62, 48000);
    m_currentDelayTime = 500.0; // msec
    m_delayBufferLength = 48000;
    m_readPointerIndex = 0;
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
      // pointer arithmetic for buffer wrap
      m_readPointerIndex %= m_delayBufferLength;

      // Use the low freq osc to modulate the delay
      int writeIndex = m_readPointerIndex +
                       ((1 + m_osc.getSample()) * m_currentDelayTime * m_sampleRate / 1000);

      // pointer arithmetic for buffer wrap
      writeIndex %= m_delayBufferLength;

      // return original plus delay
      output[sample] = input[sample] + m_level * m_delayBuffer[m_readPointerIndex++];

      // add feedback to the buffer
      m_delayBuffer[writeIndex++] = input[sample] + (output[sample] * m_feedback);
    }
  };

private:
  LowFreqOsc m_osc;
  float m_delayBuffer[48000]; // 1 second of delay buffer
  int m_sampleRate = 48000;
  float m_currentDelayTime = 0.0;
  int m_readPointerIndex;
  int m_delayBufferLength;
  float m_feedback;
  float m_level;
};