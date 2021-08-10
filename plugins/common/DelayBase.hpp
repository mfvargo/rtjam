#pragma once

#include "Effect.hpp"
#include "LowFreqOsc.hpp"
#include "BiQuad.hpp"

#define DELAY_BUFFER_SIZE 96000
#define LFO_GAIN -42.0

class DelayBase : public Effect
{
public:
  enum DelayMode
  {
    digital = 0,
    analog,
    highpass
  };

  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
    m_currentDelayTime = 0.250;
    m_feedback = 0.1;
    m_level = 0.5;
    m_drift = SignalBlock::dbToLinear(-42.0);
    m_rate = 1.4;
    m_delayMode = DelayMode::digital;
    m_writePointerIndex = 0;
    m_overallGain = 1.0;
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig(); // This is for the effect base class

    switch (m_delayMode)
    {
    case DelayMode::digital:
      m_feedbackFilter.init(BiQuadFilter::FilterType::LowPass, 10000, 1.0, 1.0, 48000);
      break;
    case DelayMode::analog:
      m_feedbackFilter.init(BiQuadFilter::FilterType::LowPass, 1250, 1.0, 1.0, 48000);
      break;
    case DelayMode::highpass:
      m_feedbackFilter.init(BiQuadFilter::FilterType::HighPass, 1500, 1.0, 1.0, 48000);
      break;
    }
    m_osc.init(LowFreqOsc::WaveShape::sineWave, m_rate, m_drift, 48000);
    m_bufferDepth = (1.0 + SignalBlock::dbToLinear(m_drift)) * m_currentDelayTime * m_sampleRate; // max delay based on depth
  }

  //  Digital Delay Effect - Signal Flow Diagram
  //
  //  Delay with modulation and filter.
  //  LPF for analog delay simulation
  //  HPF for "thinning delay"
  //
  //          ┌───────────────────────────────────────────┐
  //          │                                           │
  //          │             ┌────────────┐                ▼
  //          │    ┌────┐   │            │    ┌─────┐   ┌────┐
  //  Input───┴───►│Sum ├──►│   Delay    ├─┬─►│Level├──►│Sum ├───► Output
  //               └────┘   │            │ │  └─────┘   └────┘
  //                 ▲      └────────────┘ │
  //                 │            ▲        │
  //                 │            │        │
  //              ┌──┴───┐     ┌──┴──┐     │
  //      LPF/HPF │Filter│     │ Mod │     │
  //              └──────┘     └─────┘     │
  //                 ▲                     │
  //                 │        ┌────────┐   │
  //                 └────────┤Feedback│◄──┘
  //                          └────────┘
  //                            0-1.2
  //
  void process(const float *input, float *output, int framesize) override
  {
    // Implement the delay
    for (int sample = 0; sample < framesize; sample++)
    {
      // pointer arithmetic for buffer wrap
      m_writePointerIndex++;
      m_writePointerIndex %= m_bufferDepth;

      // Use the low freq osc to modulate the delay
      int readIndex = m_writePointerIndex -
                      ((1 + m_osc.getSample(input[sample])) * m_currentDelayTime * m_sampleRate);

      if (readIndex < 0)
      {
        readIndex += m_bufferDepth;
      }
      // pointer arithmetic for buffer wrap
      readIndex %= m_bufferDepth;

      // return original plus delay
      output[sample] = m_overallGain * (input[sample] + m_delayBuffer[readIndex] * m_level);

      // add feedback to the buffer
      m_delayBuffer[m_writePointerIndex] = input[sample] + (m_feedbackFilter.getSample(m_delayBuffer[readIndex]) * m_feedback);
    }
  };

protected:
  LowFreqOsc m_osc;
  BiQuadFilter m_feedbackFilter;
  float m_delayBuffer[DELAY_BUFFER_SIZE]; // 1 second of delay buffer
  int m_sampleRate = 48000;
  int m_bufferDepth;
  float m_currentDelayTime = 0.0;
  int m_writePointerIndex;
  float m_feedback;
  float m_level;
  float m_drift;
  float m_rate;
  float m_overallGain;
  DelayMode m_delayMode;
};