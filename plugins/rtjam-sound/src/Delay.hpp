#pragma once

#include "Effect.hpp"

class SigmaDelay : public Effect
{
public:
  void init() override{
      // Do some init stuff
  };
  void process(const float *input, float *output, int framesize) override
  {
    byPass(input, output, framesize);
  };
  float m_delayBuffer[48000]; // 1 second of delay buffer
  int m_currentDelayTime = 0;
};