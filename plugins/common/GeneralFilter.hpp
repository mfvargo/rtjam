#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class GeneralFilter : public Effect
{
public:
  json getConfig()
  {
    // Return the json for this block
    json config;
    config["name"] = "General Filter";
    config["settings"] = Effect::getConfig();
    config["settings"]["filterType"] = {{"type", "int"}, {"min", 0}, {"max", 5}, {"units", "msec"}, {"value", m_filter.m_type}};
    config["settings"]["filterFreq"] = {{"type", "float"}, {"min", 0.0}, {"max", 20000.0}, {"units", "msec"}, {"value", m_filter.m_cutoffFreq}};
    config["settings"]["filterQ"] = {{"type", "float"}, {"min", 0.1}, {"max", 20}, {"units", ""}, {"value", m_filter.m_cutBoost}};
    config["settings"]["filterCutBoost"] = {{"type", "float"}, {"min", -40.0}, {"max", 40.0}, {"units", "dB"}, {"value", m_filter.m_q}};
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]["value"]);
    
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
    setByPass(true);
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
