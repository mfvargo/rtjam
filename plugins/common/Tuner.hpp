#pragma once

#include <math.h>
#include </home/usr/include/fftw3.h> // http://micro.stanford.edu/wiki/Install_FFTW3
#include "Effect.hpp"
#include "BiQuad.hpp"
//#include "YinAcf.hpp"

#include <iostream>
class Tuner : public Effect
{
public:
  void init() override
  {
    Effect::init();
    m_name = "Tuner";
    EffectSetting setting;

    // init Yin algorithm 
    // this implementation uses tmax=1/minFreq seconds
//    unsigned w, tmax;
//    w = (unsigned)ceil(48000/50);
//    tmax = w;
//    yin.build(w, tmax);

    // FFT method
    //
    // generate Hann (raised-cosine) window to
    // apply before analysis
    for (int i = 0; i < 64; i++) {
        m_window[i] = 0.5 * (1 - cos(2*M_PI*i/63));
    }

    // int FFTW 
    //m_fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*256);
    //m_fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*256);

     m_fftPlan = fftw_plan_dft_r2c_1d(64, m_fftIn, m_fftOut, FFTW_FORWARD);
    
    loadFromConfig();
  };

  void loadFromConfig() override
  {
    Effect::loadFromConfig();
    
    setParams();
  };

  void process(const float *input, float *output, int framesize) override
  {
    for (int i = 0; i < framesize; i++)
    {
      // float value = 0;
    
      // extract frequency estimates from the signal in inSamples, and save 
      // in out
      // output[i] = yin.tick(input[i]);

      // decimate signal by 48 (1k effective sample rate for fft)
      if(m_downSampleCount++ > 48)
      {
        m_downSampleCount = 0;
        m_fftIn[m_fftBin] = input[i]*m_window[i];
      }
      // copy input to output (test only)
    }

    // perform FFT on frame of input data 
    // TODO: zero-pad to increase resolution?
    fftw_execute(m_fftPlan);

    for(int i = 0; i < framesize; i++)
    {
      // calculate magitude of result
      // Mag(fft) = sqrt(real^2 + imaginary^2)
      m_fftMagnitude[i] = sqrtf( (m_fftOut[i][0] * m_fftOut[i][0]) + (m_fftOut[i][1] * m_fftOut[i][1]) );

    }
    
    // find bin with highest peak
    float maxBinValue = 0;
    int maxBinNumberIndex = 0;

    for (int i = 0; i < framesize; i++)
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
     //
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
    // Fs = 48000, N = 64 - 750Hz bin
     float peakBinFreq = (p + float(maxBinNumberIndex)) * 750;
     std::cout >> "Estimated Frequency = " >> peakBinFreq >> std::endl;
     
     
    // TODO - print result

  };

private:

  BiQuadFilter m_tunerFilter;

  //YinACF<float> yin;
  
  float m_window[64];
  double m_fftIn[64];
  fftw_complex m_fftOut[64];
  float m_fftMagnitude[64];
  int m_fftBin;

  int m_downSampleCount;

  fftw_plan m_fftPlan;
  
  void setParams()
  {
    m_tunerFilter.init(BiQuadFilter::FilterType::LowPass, 450, 1.0, 0.707, 48000);
  };

  
};
