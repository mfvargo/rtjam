#pragma once

#include <math.h>
#include <fftw3.h>
#include "SignalBlock.hpp"
#include "BiQuad.hpp"


class TunerBlock : public SignalBlock
{
public:
  void init(/* What params do we need?*/)
  {
    m_detectedFrequency = 0;
    // generate hann (raised cosine) filter
    for (int i = 0; i < s_fftSize; i++) 
    {
        m_window[i] = 0.5 * (1 - cos(2*M_PI*i/63));
    }

     m_fftPlan = fftw_plan_dft_r2c_1d(s_fftSize, m_fftIn, m_fftOut, FFTW_ESTIMATE);

    setParams();
  
  };

  float getSample(float input) override
  {
    
    // filter and downsample/window input
    // apply 8th order IIR and store every 48th sample - Fs = 1ksps
    float value = m_tunerFilter.getSample(input);
    value = m_tunerFilter.getSample(value);
    value = m_tunerFilter.getSample(value);
    value = m_tunerFilter.getSample(value);
    value *= 48;  // add gain before downsampling
    if((++m_downSampleCount % 48) == 0)
    {
       m_fftIn[m_fftBin++] = value * m_window[m_fftBin];                
    }

   // check if fft input buffer is full 
   if( (m_fftBin % s_fftSize) == 0)
    {
      m_fftBin = 0;   // reset fft bin index for next fft
     fftw_execute(m_fftPlan);

      for(int i = 0; i < s_fftSize; i++)
      {
        // calculate magitude of result
        // Mag(fft) = sqrt(real^2 + imaginary^2)
        m_fftMagnitude[i] = sqrtf( (m_fftOut[i][0] * m_fftOut[i][0]) + (m_fftOut[i][1] * m_fftOut[i][1]) );
      }
    
      // find bin with highest peak
      float maxBinValue = m_fftMagnitude[0];
      int maxBinNumberIndex = 0;

      for (int i = 1; i < s_fftSize/2; i++)
      {
        if(m_fftMagnitude[i] > maxBinValue)
        {
          maxBinValue = m_fftMagnitude[i];
          maxBinNumberIndex = i;
        }
      } 

     // parabolic intepolation
     // p - peak location relative to maxbin (+/- 0.5 samples)
     // p = 0.5*((alpha-gamma)/(alpha-2*beta + gamma))
     // alpha -  bin to left of max bin
     // beta -  max value bin
     // gamma - bin to right of max bin
     float alpha = m_fftMagnitude[maxBinNumberIndex - 1];
     float beta = m_fftMagnitude[maxBinNumberIndex];
     float gamma = m_fftMagnitude[maxBinNumberIndex + 1];

     // float p = 0.5*((alpha-gamma)/(alpha-2*beta + gamma));
     // note - test - should be equiv solution for parabola
     float p = (gamma - alpha)/(2*(2*beta - gamma - alpha)); 

    // estimated freq = bin number (fractional) * Fs/N
    // Fs = 48000/48=1000, N = 128 - 7.8125 Hz/bin
     m_detectedFrequency = (p + float(maxBinNumberIndex)) * 1000/s_fftSize;
    
    }
    
    return input;
  };

  float getFrequency()
  {
    return m_detectedFrequency;
  }

private:
  BiQuadFilter m_tunerFilter;
  
  int m_downSampleCount;

  static const int s_fftSize = 128;
  
  float m_window[s_fftSize];
  
  double m_fftIn[s_fftSize];
  fftw_complex m_fftOut[s_fftSize];
  float m_fftMagnitude[s_fftSize];
  int m_fftBin = 0;
  fftw_plan m_fftPlan;

  float m_detectedFrequency;

  void setParams()
  {
    m_tunerFilter.init(BiQuadFilter::FilterType::LowPass, 450, 1.0, 0.707, 48000);
  };
};