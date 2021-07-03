#pragma once

#include "SignalBlock.hpp"

class BiQuadFilter : public SignalBlock
{
public:
  enum FilterType
  {
    EQ_HIGH_PASS_FILTER,
    EQ_BASS_FILTER,
    EQ_LOW_MIDRANGE_FILTER,
    EQ_MIDRANGE_FILTER,
    EQ_HIGH_MIDRANGE_FILTER,
    EQ_TREBLE_FILTER,
    EQ_LOW_PASS_FILTER,
    REVERB_LOW_PASS_FILTER,
    COMP_EQ_BAND1_FILTER,
    COMP_EQ_BAND2_FILTER,
    COMP_EQ_BAND3_FILTER,
    DELAY_TONE_LOW_PASS_FILTER,
    CHORUS_TONE_FILTER1,
    CHORUS_TONE_FILTER2,
    CHORUS_TONE_FILTER3,
    MAX_FILTER_TYPES
  };

  void init(FilterType filterType, float cutoffFreq, float cutBoost, float q)
  {
    m_type = filterType;
    m_cutoffFreq = cutoffFreq;
    m_cutBoost = cutBoost;
    m_q = q;

    switch (filterType)
    {
    case EQ_HIGH_PASS_FILTER:
      calcHighPassFilterCoefficients();
      break;

    case EQ_BASS_FILTER:
    case EQ_LOW_MIDRANGE_FILTER:
    case EQ_MIDRANGE_FILTER:
    case EQ_HIGH_MIDRANGE_FILTER:
    case EQ_TREBLE_FILTER:
    case COMP_EQ_BAND1_FILTER:
    case COMP_EQ_BAND2_FILTER:
    case COMP_EQ_BAND3_FILTER:
      calcPeakingFilterCoefficients();
      break;

    case EQ_LOW_PASS_FILTER:
    case REVERB_LOW_PASS_FILTER:
    case DELAY_TONE_LOW_PASS_FILTER:
      calcLowPassFilterCoefficients();
      break;

    case CHORUS_TONE_FILTER1:
      calcPeakingFilterCoefficients();
      break;

    default:
      break;
    }
  };

  json getConfig() override
  {
    json config = {
        {"filterType", m_type},
        {"cutoffFreq", m_cutoffFreq},
        {"cutBoost", m_cutBoost},
        {"q", m_q},
        {"a0", a0},
        {"a1", a1},
        {"a2", a2},
        {"b0", b0},
        {"b1", b1},
        {"b2", b2},
    };
    return config;
  }

  float getSample() override
  {
    return 0.0;
  };

private:
  float m_cutoffFreq, m_cutBoost, m_q;
  float a0, a1, a2, b0, b1, b2;
  float flt_A, flt_wo, flt_cos_wo, flt_sin_wo, flt_alpha;
  FilterType m_type;

  void calcHighPassFilterCoefficients()
  {
    calcIntermediateVariables(1.0, M_SQRT1_2);
    b0 = (1.0 + flt_cos_wo) / 2.0;
    b1 = -1.0 * (1.0 + flt_cos_wo);
    b2 = (1.0 + flt_cos_wo) / 2.0;
    a0 = 1.0 + flt_alpha;
    a1 = -2.0 * flt_cos_wo;
    a2 = 1.0 - flt_alpha;
    normalizeCoefficients();
  };

  void calcPeakingFilterCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    flt_alpha = flt_sin_wo / (2.0 * m_q * flt_A); // special case: alpha has term for cut/boost (not in LPF/HPF)
    a0 = 1.0 + (flt_alpha / flt_A);
    a1 = -2.0 * flt_cos_wo;
    a2 = 1.0 - (flt_alpha / flt_A);
    b0 = (1.0 + (flt_alpha * flt_A)); // * gainlinear
    b1 = -2.0 * flt_cos_wo;           // * gainlinear
    b2 = 1.0 - (flt_alpha * flt_A);   // * gainlinear
    normalizeCoefficients();
  };

  void calcLowPassFilterCoefficients()
  {
    calcIntermediateVariables(1.0, M_SQRT1_2);
    b0 = (1.0 - flt_cos_wo) / 2.0;
    b1 = 1.0 - flt_cos_wo;
    b2 = (1.0 - flt_cos_wo) / 2.0;
    a0 = 1.0 + flt_alpha;
    a1 = -2.0 * flt_cos_wo;
    a2 = 1.0 - flt_alpha;
    normalizeCoefficients();
  };

  void calcIntermediateVariables(float fltCutBoost, float fltQ)
  {
    flt_A = pow(10.0, (fltCutBoost / 40.0));
    flt_wo = 2.0 * M_PI * (m_cutoffFreq / 48000);
    flt_cos_wo = cos(flt_wo);
    flt_sin_wo = sin(flt_wo);
    flt_alpha = flt_sin_wo / (2.0 * fltQ);
  }

  void normalizeCoefficients()
  {
    b2 /= a0;
    b1 /= a0;
    b0 /= a0;
    a2 /= a0;
    a1 /= a0;
    a0 = 1.0;
  }
};
