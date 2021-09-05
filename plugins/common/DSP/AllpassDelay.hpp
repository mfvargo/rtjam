#pragma once

#include "SignalBlock.hpp"
#include <vector>

class AllpassDelay : public SignalBlock
{
public:
    void init(float delayLength, float apGain)
    {
        // initialize the delay line and coeffs
        m_apGain = apGain;
        m_apDelayIndex = 0;
        m_apDelayLength = delayLength;
        m_apDelayLine.resize(m_apDelayLength);
    };

    float getSample(float input) override
    {
        // all-pass delay or "stretched" all-pass
        float delayOut = m_apDelayLine[m_apDelayIndex]; // read in end of delay line
        float delayIn = input + delayOut * m_apGain;    // delay in - sum of input and fb path
        m_apDelayLine[m_apDelayIndex] = delayIn;        // write to delay line - new delay sample
        delayOut += delayIn * (-1.0 * m_apGain);        // ap out = sum of delay out and ff path
        ++m_apDelayIndex %= m_apDelayLength;            // wrap the index around the buffer
        return delayOut;
    };

public:
private:
    float m_apGain;
    std::vector<float> m_apDelayLine;
    int m_apDelayLength;
    int m_apDelayIndex;
};
