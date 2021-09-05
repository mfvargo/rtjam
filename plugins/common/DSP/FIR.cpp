#pragma once

#include "SignalBlock.hpp"

class FirFilter : public SignalBlock
{
public:
  void init(float FirCoeffs[], int N)
  {

    m_firCoeffs.resize(N);
    m_tap.resize(N);
    m_N = N;

    for (unsigned int i = 0; i < N; i++)
    {
      m_firCoeffs[i] = FirCoeffs[i];
    }
  };

  float getSample(float input) override
  {

    // Copy the new sample in
    m_tap[m_tapIndex++] = input;
    if (m_tapIndex >= m_N)
    {
      m_tapIndex = 0;
    }

    // convolution sum
    // convolve input function with the impulse response
    //

    m_sum = 0;
    for (int i = 0; i < m_N; i++)
    {
      m_sum += m_tap[m_tapIndex++] * m_firCoeffs[m_N - i];

      if (m_tapIndex >= m_N)
      {
        m_tapIndex = 0;
      }
    }

    return m_sum;
  };

private:
  std::vector<float> m_firCoeffs;
  std::vector<float> m_tap;
  unsigned int m_tapIndex;
  unsigned int m_N;
  float m_sum;
};
