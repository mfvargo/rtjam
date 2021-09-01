#pragma once

#include "Effect.hpp"
#include "BiQuad.hpp"
#include "PeakDetector.hpp"

class EnvelopeFilterBase : public Effect
{
public:
  enum FilterType
  {
    LPF,
    BPF,
    HPF
  };

  // This init is called from the derived class
  void init() override
  {
    // Setup base class stuff (bypass etc)
    Effect::init();
  };

  void loadFromConfig() override
  {
    // Read the settings from the map and apply them to our copy of the data.
    Effect::loadFromConfig();

    // init the attack/release and filter signal blocks
    m_envelopeOut.init(m_envAttack, m_envRelease, 48000);
    switch (m_filterType)
    {
    case BPF:
      m_envelopeFilter.init(BiQuadFilter::BandPass, m_envFreq, 1.0, m_envResonance, 48000);
      break;

    case LPF:
      m_envelopeFilter.init(BiQuadFilter::LowPass, m_envFreq, 1.0, m_envResonance, 48000);
      break;

    case HPF:
      m_envelopeFilter.init(BiQuadFilter::HighPass, m_envFreq, 1.0, m_envResonance, 48000);
      break;

    default:
      break;
    }
  };
  //
  //  Envelope Filter Effect - Block Diagram
  //
  //
  //                              +----------+
  //            ----------------->| Dry Gain |------------------------
  //            |                 |          |                       |
  //            |                 +----------+                       |
  //            |                                                    |
  //            |                                                    |
  //            |               +---------------+                    |
  //            |               | Biquad Filter |                    |
  //            |               |      (2)      |                    v
  // Input -------------------->| - 4 pole      |     +-----+    +------+
  //               |            | - LPF/BPF/HPF |---->|level|--->| sum  |---> Output
  //               |            |               |     +-----+    +------+
  //               |            |               |
  //               |            +---------------+
  //               |                     ^
  //               |                     |
  //               v              +--------------+
  //        +----------------+    |Coeff Control |
  //        | Peak Detector  |    |              |
  //        |                |--->| Sens/Freq/Q  |
  //        | -attack/release|    |              |
  //        +----------------+    | - Fs/4       |
  //                              +--------------+
  //
  //
  void process(const float *input, float *output, int framesize) override
  {

    // simple envelope filter implementation
    //
    for (int sample = 0; sample < framesize; sample++)
    {

      float value = m_envelopeOut.getSample(input[sample]); // calculate magnitude of incoming signal
      value = (value * m_envSensitivity * 20) + m_envFreq;  // apply gain to envelope, add in freq knob (start freq)

      if (++m_envelopeSample %= s_updateRate)
      {
        switch (m_filterType)
        {
        case BPF:
          m_envelopeFilter.updateCoeffs(BiQuadFilter::BandPass, value, 1.0, m_envResonance, 48000);
          break;

        case LPF:
          m_envelopeFilter.updateCoeffs(BiQuadFilter::LowPass, value, 1.0, m_envResonance, 48000);
          break;

        case HPF:
          m_envelopeFilter.updateCoeffs(BiQuadFilter::HighPass, value, 1.0, m_envResonance, 48000);
          break;

        default:
          break;
        }
      }

      // apply 4th order dynamic filter
      value = 0.5 * m_envelopeFilter.getSample(input[sample]);
      value = m_envelopeFilter.getSample(value);

      if (m_filterType == FilterType::BPF) // BPF and HPF need make-up gain
      {
        value *= 12;
      }
      else if (m_filterType == FilterType::HPF)
      {
        value *= 2;
      }

      output[sample] = (value * m_filterLevel);     // output level
      output[sample] += m_dryLevel * input[sample]; // output + dry
    }
  };

private:
  BiQuadFilter m_envelopeFilter;
  PeakDetector m_envelopeOut;

  static const int s_updateRate = 4;
  int m_envelopeSample = 0;

protected:
  float m_envFreq;
  float m_envResonance;

  float m_envSensitivity;

  int m_filterType = BPF;

  float m_envAttack;
  float m_envRelease;

  float m_filterLevel;
  float m_dryLevel;
};