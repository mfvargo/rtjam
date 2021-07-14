#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class HighPassFilter : public Effect
{
public:
  void init() override
  {
    Effect::init();
    m_name = "DC Offset HighPass";
    m_filter.init(BiQuadFilter::FilterType::HighPass, 2.0, 1.0, 1.0, 48000);
    loadFromConfig();
  }

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
  }
  void process(const float *input, float *output, int framesize) override
  {
    m_filter.getBlock(input, output, framesize);
  };

private:
  double tap1, tap2, a1, b1, b0;
  BiQuadFilter m_filter;
};
