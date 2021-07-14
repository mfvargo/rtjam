#pragma once

#include "SignalBlock.hpp"

class LowFreqOsc : public SignalBlock
{
public:
  enum WaveShape
  {
    sineWave,
    squareWave,
    rampWave,
  };

  void init(WaveShape shape, float freq, float amplitude, int sampleRate)
  {
    m_waveShape = shape;
    m_frequency = freq;
    m_phaseIncrement = 2 * M_PI * freq / sampleRate;
    m_amplitude = amplitude;
    m_sampleRate = sampleRate;
    m_currentPhase = 0;
  };

  json getConfig() override
  {
    // Return the json for this block
    json config = {
        {"shape", m_waveShape},
        {"freq", m_frequency},
        {"amplitude", m_amplitude}};
    return config;
  };

  void setConfig(json config)
  {
    init(config["shape"], config["freq"], config["amplitude"], 48000);
  };

  float getSample(float input) override
  {
    float value = 0.0;
    // Right now this is the only wave form...
    switch (m_waveShape)
    {
    case sineWave:
      value = m_amplitude * std::sin(m_currentPhase);
      break;
    case squareWave:
      if (m_currentPhase < M_PI)
        value = m_amplitude;
      else
        value = -m_amplitude;
      break;
    case rampWave:
      value = (m_currentPhase / (2 * M_PI) * m_amplitude);
      break;
    }
    m_currentPhase += m_phaseIncrement;
    if (m_currentPhase >= 2 * M_PI)
    {
      m_currentPhase -= 2 * M_PI;
    }
    return value;
  };

private:
  float m_phaseIncrement;
  float m_frequency;
  float m_amplitude;
  int m_sampleRate;
  float m_currentPhase;
  WaveShape m_waveShape;
};