#pragma once

#include "Effect.hpp"

class SigmaDelay : public Effect
{
public:
  void init() override
  {
    // Do some init stuff
    // setByPass(true);
    m_currentDelayTime = 500.0; // msec
    m_delayBufferLength = 48000;
    m_readPointerIndex = 0;
    m_writePointerIndex = m_currentDelayTime * m_sampleRate / 1000;
    printf("write index: %d\n", m_writePointerIndex);
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
      if (m_writePointerIndex >= m_delayBufferLength)
      {
        m_writePointerIndex = 0;
      }
      m_delayBuffer[m_writePointerIndex++] = input[sample];
      if (m_readPointerIndex >= m_delayBufferLength)
      {
        m_readPointerIndex = 0;
      }
      output[sample] = input[sample] + m_delayBuffer[m_readPointerIndex++];
    }
    // byPass(input, output, framesize);
  };

private:
  float m_delayBuffer[48000]; // 1 second of delay buffer
  int m_sampleRate = 48000;
  float m_currentDelayTime = 0.0;
  int m_writePointerIndex;
  int m_readPointerIndex;
  int m_delayBufferLength;
};