#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class HighPassFilter : public Effect
{
public:
  json getConfig()
  {
    // Return the json for this block
    json config;
    config["name"] = "DC Offset HighPass";
    config["settings"] = Effect::getConfig();
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]["value"]);
  }

  void init() override
  {
    setByPass(true);
    m_filter.init(BiQuadFilter::FilterType::EQ_HIGH_PASS_FILTER, 2.0, 1.0, 1.0, 48000);
  }
  void process(const float *input, float *output, int framesize) override
  {
    m_filter.getBlock(input, output, framesize);
  };

private:
  double tap1, tap2, a1, b1, b0;
  BiQuadFilter m_filter;
};
