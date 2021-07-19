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
      if (!m_chain[i]->getByPass())
      {
        m_chain[i]->doProcess(inBuff, outBuff, framesize);
        // Now swap the pointers
        float *temp = inBuff;
        inBuff = outBuff;
        outBuff = temp;
      }
    }
    memcpy(output, inBuff, framesize * sizeof(float));
  };

  json getChainConfig(std::string name, int channel)
  {
    json effects = json::array();
    for (int i = 0; i < m_chain.size(); i++)
    {
      json config = m_chain[i]->getSettings();
      config["index"] = i;
      effects.push_back(config);
    }
    json rval = {
        {"name", name},
        {"channel", channel},
        {"effects", effects}};
    return rval;
  };

  int size()
  {
    return m_chain.size();
  };

  void toggleEffect(int i)
  {
    if (i < m_chain.size())
    {
      json setting;
      setting["name"] = "bypass";
      setting["value"] = !m_chain[i]->getByPass();
      m_chain[i]->setSettingValue(setting);
    }
  };

  Effect *getEffect(int i)
  {
    return m_chain[i];
  };

private:
  std::vector<Effect *> m_chain;
};