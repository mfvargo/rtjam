#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"

#include <iostream>
class ToneStack : public Effect
{
public:
  json getConfig()
  {
    // Return the json for this block
    json config;
    config["name"] = "Tone Stack";
    config["settings"] = Effect::getConfig();
    config["settings"]["treble"] = {{"type", "float"}, {"min", -10.0}, {"max", 10.0}, {"units", "linear"}, {"value", m_treble}};
    config["settings"]["mid"] = {{"type", "float"}, {"min", -10.0}, {"max", 10.0}, {"units", "linear"}, {"value", m_mid}};
    config["settings"]["bass"] = {{"type", "float"}, {"min", -10.0}, {"max", 10.0}, {"units", "linear"}, {"value", m_bass}};
    return config;
  };

  void setConfig(json config)
  {
    setByPass(config["bypass"]["value"]);
    m_treble = config["treble"]["value"];
    m_mid = config["mid"]["value"];
    m_bass = config["bass"]["value"];
    setParams();
  }

  void init() override
  {
    m_treble = 0.0;
    m_mid = 0.0;
    m_bass = 0.0;
    setParams();
  };

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = 0.3333 * input[i];
      output[i] += m_bass / 10 * m_fBass.getSample(input[i]);
      output[i] += m_mid / 10 * m_fMid.getSample(input[i]);
      output[i] += m_treble / 10 * m_fHigh.getSample(input[i]);
    }
  };

private:
  float m_treble, m_mid, m_bass;
  BiQuadFilter m_fBass, m_fMid, m_fHigh;

  void setParams()
  {
    // TODO: figure out how to chain some BiQuads to do this.
    m_fBass.init(BiQuadFilter::FilterType::EQ_BASS_FILTER, 200, 1.0, 1.0, 48000);
    m_fMid.init(BiQuadFilter::FilterType::EQ_MIDRANGE_FILTER, 700, 1.0, 1.0, 48000);
    m_fHigh.init(BiQuadFilter::FilterType::EQ_TREBLE_FILTER, 2000, 1.0, 1.0, 48000);
  };
};
