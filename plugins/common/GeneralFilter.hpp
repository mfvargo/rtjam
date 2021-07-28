#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class GeneralFilter : public Effect
{
public:


  void setConfig(json config)
  {
 //   setByPass(config["bypass"]["value"]);
    
    m_filter.init(
    config["filterType"]["value"],
    config["filterFreq"]["value"],
    config["filterCutBoost"]["value"],
    config["filterQ"]["value"],
    48000
    );

 }

  void init() override
  {
//    setByPass(true);
    m_filter.init(BiQuadFilter::FilterType::HighPass, 2.0, 1.0, 1.0, 48000);
  }
  void process(const float *input, float *output, int framesize) override
  {
    m_filter.getBlock(input, output, framesize);
  };

private:
  double tap1, tap2, a1, b1, b0;
  BiQuadFilter m_filter;
};
