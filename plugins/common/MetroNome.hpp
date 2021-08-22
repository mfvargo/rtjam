#pragma once

#include "SignalBlock.hpp"

class MetroNome : public SignalBlock
{
public:
  enum SoundType
  {
    clave,
    beep,
    boop,
  };

  void init(SoundType stype, int tempo, float amplitude, int sampleRate)
  {
    m_soundType = stype;
    m_tempo = tempo;
    m_amplitude = amplitude;
    m_sampleRate = sampleRate;
    m_currentPhase = 0;
  };

  float getSample(float input) override
  {
    float value = input;
    // Right now this is the only wave form...
    switch (m_soundType)
    {
    case clave:
      break;
    case beep:
      break;
    case boop:
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
  float m_amplitude;
  int m_sampleRate;
  float m_currentPhase;
  int m_tempo;
  SoundType m_soundType;
};