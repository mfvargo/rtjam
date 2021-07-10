#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"

#include <iostream>
class Tremelo : public Effect
{
public:
  json getConfig()
  {
    // Return the json for this block
    json config;
    config["name"] = "Tremelo";
    config["settings"] = Effect::getConfig();
    config["settings"]["depth"] = {{"type", "float"}, {"min", -40.0}, {"max", -3.0}, {"units", "dB"}, {"value", m_depth}};
    config["settings"]["rate"] = {{"type", "float"}, {"min", 0.0}, {"max", 10.0}, {"units", "linear"}, {"value", m_rate}};
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]["value"]);
    m_depth = config["depth"]["value"];
    m_rate = config["rate"]["value"];
    setParams();
  }

  void init() override
  {
    m_depth = 0.4;
    m_rate = 1.2;
    setParams();
  };

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = (1 + m_osc.getSample(1.0)) * input[i];
    }
  };

private:
  LowFreqOsc m_osc;
  float m_depth, m_rate;

  void setParams()
  {
    m_osc.init(LowFreqOsc::WaveShape::sineWave, m_rate, m_depth, 48000);
  };
};
