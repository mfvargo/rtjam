#pragma once

#include "EffectFactory.hpp"
#include <thread>

class PedalBoard
{
public:
  PedalBoard()
  {
    m_outputDCremoval.init(BiQuadFilter::FilterType::HighPass, 2.0, 1.0, 1.0, 48000);
  }
  // Initialize the pedal board. config should be a json::array() of pedals to construct
  // For an empty pedalboard, just pass in json::array()
  void init(json config)
  {
    markUnstable();
    std::this_thread::sleep_for(std::chrono::microseconds(2000));
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
    markStable();
  }

  // This will insert a new Pedal on the board
  void insertPedal(int idx, string pedalName)
  {
    markUnstable();
    json config = json::object();
    config["name"] = pedalName;
    config["settings"] = json::array();
    Effect *pEffect = EffectFactory::manufacture(config);
    if (pEffect != NULL)
    {
      if (idx >= m_chain.size())
      {
        m_chain.push_back(pEffect);
      }
      else
      {
        m_chain.insert(m_chain.begin() + idx, pEffect);
      }
    }
    markStable();
  }

  // This will delete the pedal at the specified index
  void deletePedal(int idx)
  {
    markUnstable();
    if (idx < m_chain.size())
    {
      Effect *pEffect = m_chain[idx];
      m_chain.erase(m_chain.begin() + idx);
      delete pEffect;
    }
    markStable();
  }

  // This will move a pedal from one idx to another
  void movePedal(int fromIdx, int toIdx)
  {
    // Check values of fromIdx and toIdx to see if they make sense
    if (fromIdx >= m_chain.size() || toIdx > m_chain.size() || fromIdx == toIdx)
      return;

    // Do the thing
    markUnstable();

    int size = m_chain.size();

    Effect *pEffect = m_chain[fromIdx];
    m_chain.erase(m_chain.begin() + fromIdx);
    // to put it on the end
    if (toIdx == size)
    {
      m_chain.push_back(pEffect);
    }
    else
    {
      m_chain.insert(m_chain.begin() + toIdx, pEffect);
    }
    markStable();
  }

  // This is the way to invoke the pedalboard on a block of samples
  void process(const float *input, float *output, int framesize)
  {
    if (m_stable)
      processBlock(input, output, framesize);
    else
      memmove(output, input, sizeof(float) * framesize);
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
    if (idx < m_chain.size())
    {
      m_chain[idx]->setSettingValue(setting);
    }
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
  void markUnstable()
  {
    m_stable = false;
    std::this_thread::sleep_for(std::chrono::microseconds(10000));
  }
  void markStable()
  {
    m_stable = true;
  }

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

    m_outputDCremoval.getBlock(inBuff, output, framesize);
  };

private:
  bool m_stable;
  std::vector<Effect *> m_chain;
  BiQuadFilter m_outputDCremoval;
};