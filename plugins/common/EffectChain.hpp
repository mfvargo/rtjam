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
      m_chain[i]->doProcess(inBuff, outBuff, framesize);
      // Now swap the pointers
      float *temp = inBuff;
      inBuff = outBuff;
      outBuff = temp;
    }
    memcpy(output, inBuff, framesize * sizeof(float));
  };

  json getChainConfig(std::string name)
  {
    json effects = json::array();
    for (int i = 0; i < m_chain.size(); i++)
    {
      effects.push_back(m_chain[i]->getConfig());
    }
    json rval = {
        {"name", name},
        {"effects", effects}};
    return rval;
  }
  void toggleEffect(int i)
  {
    if (i < m_chain.size())
    {
      m_chain[i]->setByPass(!m_chain[i]->getByPass());
    }
  }

private:
  std::vector<Effect *> m_chain;
};