#pragma once

#include "EffectFactory.hpp"
#include <thread>

class PedalBoard
{
public:
  // Initialize the pedal board. config should be a json::array() of pedals to construct
  // For an empty pedalboard, just pass in json::array()
  void init(json config)
  {
    m_stable = false;
    std::this_thread::sleep_for(std::chrono::microseconds(10000));
    // Clear out any existing pedals
    for (auto &effect : m_chain)
    {
      delete effect;
    }
    m_chain.clear();

    // Now load up new pedals from factory
    for (auto &effectJson : config)
    {
      Effect *pEffect = EffectFactory::manufacture(effectJson);
      if (pEffect != NULL)
      {
        m_chain.push_back(pEffect);
      }
    }
    m_stable = true;
  }

  // This is the way to invoke the pedalboard on a block of samples
  void process(const float *input, float *output, int framesize)
  {
    if (m_stable)
      processBlock(input, output, framesize);
    else
      memcpy(output, input, sizeof(float) * framesize);
  }

  // this will dump out the json for all the pedasl
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

  // This will apply a setting to a particular pedal
  bool setEffectSetting(json setting, int idx)
  {
    bool rval = false;
    // Check index bounds
    m_stable = false;
    if (idx < m_chain.size())
    {
      m_chain[idx]->setSettingValue(setting);
    }
    m_stable = true;
    return rval;
  }

  // This will toggle the bypass for a pedal
  void toggleEffect(int idx)
  {
    if (idx < m_chain.size())
    {
      json setting;
      setting["name"] = "bypass";
      setting["value"] = !m_chain[idx]->getByPass();
      m_chain[idx]->setSettingValue(setting);
    }
  }

private:
  void processBlock(const float *input, float *output, int framesize)
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

private:
  bool m_stable;
  std::vector<Effect *> m_chain;
};