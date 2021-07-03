#pragma once

#include "Effect.hpp"

class EffectChain
{
public:
  void push(Effect *effect)
  {
    m_chain.push_back(effect);
  }
  void clear()
  {
    m_chain.clear();
  }
  void process(const float *input, float *output, int framesize)
  {
    float ping[framesize];
    float pong[framesize];

    float *inBuff = ping;
    float *outBuff = pong;

    // Copy channel 1 into local buffer
    memcpy(inBuff, input, framesize * sizeof(float));

    // Loop through all the effects for channel 1 and process them
    for (int i = 0; i < m_chain.size(); i++)
    {
      m_chain[i]->process(inBuff, outBuff, framesize);
      // Now swap the pointers
      float *temp = inBuff;
      inBuff = outBuff;
      outBuff = temp;
    }
    memcpy(output, inBuff, framesize * sizeof(float));
  };

private:
  std::vector<Effect *> m_chain;
};