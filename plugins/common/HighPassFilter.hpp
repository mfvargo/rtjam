#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class HighPassFilter : public Effect
{
public:
  json getConfig() override
  {
    // Return the json for this block
    json config = {};
    return config;
  };

  void init() override
  {
    setByPass(true);
    m_filter.init(BiQuadFilter::FilterType::EQ_HIGH_PASS_FILTER, 2.0, 1.0, 1.0);
    std::cout << m_filter.getConfig().dump(2) << std::endl;
    tap1 = 0.0;
    tap2 = 0.0;
    a1 = -0.996863331833438;
    b0 = 0.998431665916719;
    b1 = -0.998431665916719;
  }
  void process(const float *input, float *output, int framesize) override
  {
    if (getByPass())
    {
      return byPass(input, output, framesize);
    }
    for (int i = 0; i < framesize; i++)
    {
      output[i] = b0 * input[i] + b1 * tap1 - a1 * tap2;
      tap1 = input[i];
      tap2 = output[i];
    }
  };

private:
  double tap1, tap2, a1, b1, b0;
  BiQuadFilter m_filter;
};
