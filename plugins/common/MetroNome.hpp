#pragma once
#include <iostream>
#include <math.h>

class MetroNome
{
public:
  enum SoundType
  {
    clave,
    beep,
    boop,
  };

  void init(SoundType stype, float amplitude)
  {
    m_soundType = stype;
    m_amplitude = amplitude;
    m_lastBeat = 0;
    m_pulseSampleCount = 0;
    fillBeep(m_beep, 440, 2400);
    fillBeep(m_boop, 220, 2400);
  };

  void getBlock(int thisBeat, float *output, int framesize)
  {
    for (int i = 0; i < framesize; i++)
    {
      if (m_lastBeat != thisBeat)
      {
        // rising edge of a new beat
        m_pulseSampleCount = 0;
        m_lastBeat = thisBeat;
      }
      float value = 0;
      float *beep = m_boop;
      if (thisBeat == 0)
        beep = m_beep;

      if (m_pulseSampleCount < 2400)
        value = m_amplitude * beep[m_pulseSampleCount++];

      output[i] = value;
    }
  }

private:
  void fillBeep(float *output, float freq, int numSamps)
  {
    float phase = 0;
    float phaseIncrement = 2 * M_PI * freq / 48000;
    for (int i = 0; i < numSamps; i++)
    {
      // windowed sin wave
      output[i] = 0.5 * (1 - cos(2 * M_PI * i / (numSamps))) * std::sin(phase);
      phase += phaseIncrement;
      if (phase >= 2 * M_PI)
      {
        phase -= 2 * M_PI;
      }
    }
  }

  float m_beep[2400];
  float m_boop[2400];
  float m_amplitude;
  int m_lastBeat;
  int m_pulseSampleCount;
  SoundType m_soundType;
};