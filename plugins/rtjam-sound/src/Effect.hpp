#pragma once

class Effect
{
public:
  virtual void init() = 0;
  // void setParam();
  virtual void process(const float *input, float *output, int framesize) = 0;
  void byPass(const float *input, float *output, int framesize)
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = input[i];
    }
  }
  void setByPass(bool bypass)
  {
    m_byPass = bypass;
  }
  bool getByPass()
  {
    return m_byPass;
  }

private:
  bool m_byPass = false;
  // std::vector<SignalBlocks> m_blocks;
};