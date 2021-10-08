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
    BandPass,
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

    // test only - output coeffs in MATLAB format for plotting
    //std::cout << "b = [" << b0 << " " << b1 << " " << b2 << "]" << std::endl;
    //std::cout << "a = [ 1 " << a1 << " " << a2 << "]" << std::endl;

  };


  float getSample(float input) override
  {
    double value = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = (double)input;
    y2 = y1;
    y1 = value;
    return (float)value;
  };


void updateCoeffs(FilterType filterType, float cutoffFreq, float cutBoost, float q, int sampleRate)
  {
    m_sampleRate = sampleRate;
    m_type = filterType;
    m_cutoffFreq = cutoffFreq;
    m_cutBoost = cutBoost;
    m_q = q;

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

    case BandPass:
      calcBandPassFilterCoefficients();
      break;

    default:
      break;
    }

  };




public:
  float m_cutoffFreq, m_cutBoost, m_q, m_sampleRate;
  FilterType m_type;

private:
  double a0, a1, a2, b0, b1, b2;
  double A, omega, cos_omega, sin_omega, alpha;
  double x1, x2, y1, y2;

  void calcHighPassCoefficients()
  {
    calcIntermediateVariables(1.0, m_q);
    b0 = (1.0 + cos_omega) / 2.0;
    b1 = -1.0 * (1.0 + cos_omega);
    b2 = (1.0 + cos_omega) / 2.0;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - alpha;
    normalizeCoefficients();
  };

  void calcPeakingCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = sin_omega / (2.0 * m_q * A); // special case: alpha has term for cut/boost (not in LPF/HPF)
    a0 = 1.0 + (alpha / A);
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - (alpha / A);
    b0 = (1.0 + (alpha * A)); // * gainlinear
    b1 = -2.0 * cos_omega;    // * gainlinear
    b2 = 1.0 - (alpha * A);   // * gainlinear
    normalizeCoefficients();
  };

  void calcLowPassCoefficients()
  {
    calcIntermediateVariables(1.0, m_q);
    b0 = (1.0 - cos_omega) / 2.0;
    b1 = 1.0 - cos_omega;
    b2 = (1.0 - cos_omega) / 2.0;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - alpha;
    normalizeCoefficients();
  };

  void calcLowShelfCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = (sin_omega / 2.0) * sqrt((A + (1/A))*((1/m_q)-1) + 2); // special case for shelving filter
    b0 = A*((A + 1.0) - (A -1.0)*cos_omega + (2.0*sqrt(A) * alpha)); 
    b1 = 2.0*A*((A - 1.0) - (A +1.0)*cos_omega);           
    b2 = A*((A + 1.0) - (A -1.0)*cos_omega - (2.0*sqrt(A) * alpha));   
    a0 = (A + 1.0) + (A -1.0)*cos_omega + (2.0*sqrt(A) * alpha);
    a1 = -2.0*((A -1.0) + (A + 1.0)*cos_omega);
    a2 = (A + 1.0) + (A - 1.0)*cos_omega - (2.0*sqrt(A) * alpha);
    normalizeCoefficients();
  };

  void calcHighShelfCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = (sin_omega / 2.0) * sqrt((A + (1/A))*((1/m_q)-1) + 2); // special case for shelving filter
    b0 = A*((A + 1.0) + (A -1.0)*cos_omega + (2.0*sqrt(A) * alpha)); 
    b1 = -2.0*A*((A - 1.0) + (A +1.0)*cos_omega);           
    b2 = A*((A + 1.0) + (A -1.0)*cos_omega - (2.0*sqrt(A) * alpha));   
    a0 = (A + 1.0) - (A -1.0)*cos_omega + (2.0*sqrt(A) * alpha);
    a1 = 2.0*((A -1.0) - (A + 1.0)*cos_omega);
    a2 = (A + 1.0) - (A - 1.0)*cos_omega - (2.0*sqrt(A) * alpha);
    normalizeCoefficients();
  };

  void calcAllPassCoefficients()
  {

    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = sin_omega / (2.0 * m_q * A); // special case: alpha has term for cut/boost (not in LPF/HPF)
    b0 = 1.0 - alpha;
    b1 = -2.0 * cos_omega;
    b2 = 1.0 + alpha;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - alpha;
    normalizeCoefficients();
  };

  void calcNotchFilterCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = sin_omega / (2.0 * m_q * A); // special case: alpha has term for cut/boost (not in LPF/HPF)
    b0 = 1.0;
    b1 = -2.0 * cos_omega;
    b2 = 1.0;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - alpha;
    normalizeCoefficients();
  };

    void calcBandPassFilterCoefficients()
  {
    calcIntermediateVariables(m_cutBoost, m_q);
    alpha = sin_omega / (2.0 * m_q * A); // special case: alpha has term for cut/boost (not in LPF/HPF)
    b0 = alpha; // *m_q;
    b1 = 0.0;
    b2 = -1*alpha; // *m_q;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_omega;
    a2 = 1.0 - alpha;
    normalizeCoefficients();
  };

  void calcIntermediateVariables(float fltCutBoost, float fltQ)
  {
    A = pow(10.0, (fltCutBoost / 40.0));
    omega = 2.0 * M_PI * (m_cutoffFreq / m_sampleRate);
    cos_omega = cos(omega);
    sin_omega = sin(omega);
    alpha = sin_omega / (2.0 * fltQ);
   
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
