#pragma once

#include "json.hpp"

using json = nlohmann::json;

class Effect
{
public:
  virtual void init() = 0;
  virtual json getConfig()
  {
    // Return the json for this block
    json config = {
        {"bypass", {{"type", "boolean"}, {"value", getByPass()}}},
    };
    return config;
  };

  virtual void setConfig(json config) = 0;

  void doProcess(const float *input, float *output, int framesize)
  {
    // check for bypass
    if (getByPass())
    {
      return byPass(input, output, framesize);
    }
    return process(input, output, framesize);
  }
  // void setParam();
  virtual void process(const float *input, float *output, int framesize) = 0;
  // Bypass set/get
  void setByPass(bool bypass)
  {
    m_byPass = bypass;
  }
  bool getByPass()
  {
    return m_byPass;
  }

private:
  bool m_byPass = false;

  // Code to bypass this effect
  void byPass(const float *input, float *output, int framesize)
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = input[i];
    }
  }
};