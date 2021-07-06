#pragma once

#include "SignalBlock.hpp"

class BiQuadFilter : public SignalBlock
{
public:
  enum FilterType
  {
    LowPass,
    HighPass,
    Peaking,
    LowShelf,
    HighShelf,
    AllPass,
    Notch,
    MAX_FILTER_TYPES
  };

  void init(FilterType filterType, float cutoffFreq, float cutBoost, float q, int sampleRate)
  {
    m_sampleRate = sampleRate;
    m_type = filterType;
    m_cutoffFreq = cutoffFreq;
    m_cutBoost = cutBoost;
    m_q = q;
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;

    switch (filterType)
    {
    case LowPass:
      calcLowPassCoefficients();
      break;

    case HighPass:
      calcHighPassCoefficients();
      break;

    case Peaking:
      calcPeakingCoefficients();
      break;

    case LowShelf:
      calcLowShelfCoefficients();
      break;

    case HighShelf:
      calcHighShelfCoefficients();
      break;

    case AllPass:
      calcAllPassCoefficients();
      break;

    case Notch:
      calcNotchFilterCoefficients();
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
    };
    return config;
  }

  float getSample(float input) override
  {
    float value = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = value;
    return value;
  };

private:
  float m_cutoffFreq, m_cutBoost, m_q, m_sampleRate;
  float a0, a1, a2, b0, b1, b2;
  float flt_A, flt_wo, flt_cos_wo, flt_sin_wo, flt_alpha;
  FilterType m_type;
  float x1, x2, y1, y2;

  void calcHighPassCoefficients()
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

  void calcPeakingCoefficients()
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

  void calcLowPassCoefficients()
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

  void calcLowShelfCoefficients()
  {
    // TODO: need to code this
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

  void calcHighShelfCoefficients()
  {
    // TODO: need to code this
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

  void calcAllPassCoefficients()
  {
    // DFM TODO: need to code this
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

  void calcNotchFilterCoefficients()
  {
    // TODO: need to code this
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

  void calcIntermediateVariables(float fltCutBoost, float fltQ)
  {
    flt_A = pow(10.0, (fltCutBoost / 40.0));
    flt_wo = 2.0 * M_PI * (m_cutoffFreq / m_sampleRate);
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
