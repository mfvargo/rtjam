// HP Filter Function for RTJam
//
// Implements a 1st order Butterworth High-pass filter
// using the following standard difference equation:
// y(n) = b0*x(n) + b1*x(n-1) - a1*y(n-1))
//
// where b0, b1, and a1 are filter coefficients calculated in MATLAB
// using [b,a] = butter(1,.001,'high')
//
// b0 = 0.9984, b1 = -0.9984, a1 = -.9969
//
// x(n) - current input sample
// y(n) - current output sample
// x(n-1) - filter tap 1 - input sample delayed by 1
// y(n-1) - filter tap 2 - output sample delayed by 2
//
//

#pragma once

#include "JamNetStuff.hpp"

// filter coefficients from MATLAB
#define B0 0.9984
#define B1 -0.9984
#define A1 0.9969

namespace JamNetStuff
{
  class HighPassFilter
  {
  public:
    HighPassFilter()
    {
      in_prev = 0.0;
      out_prev = 0.0;
    }
    void filter(float *output, const float *input, uint32_t framesize)
    {
      for (uint32_t i = 0; i < framesize; i++)
      {
        out_prev = B0 * input[i] + B1 * in_prev - A1 * out_prev;
        in_prev = input[i];
        output[i] = out_prev * 2.0;
        output[i] = input[i]; // Uncomment to do pass through
      }
    }

  private:
    float in_prev;
    float out_prev;
  };
}
