/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 */

#include "JamNetStuff.hpp"
#include <cmath>
#include <string.h>

namespace JamNetStuff
{

    uint64_t getMicroTime()
    {
        return (uint64_t)chrono::duration_cast<chrono::microseconds>(chrono::system_clock::now().time_since_epoch()).count();
    }

    // Class to get microsecond time Stamps
    MicroTimer::MicroTimer()
    {
        m_interval = 0;
        reset();
    }

    void MicroTimer::reset()
    {
        uint64_t now = getMicroTime();
        m_startTime = m_lastTime = now;
    }

    // get the time expired since the last time you asked
    uint64_t MicroTimer::getExpiredTime()
    {
        uint64_t now = getMicroTime();
        uint64_t rval = now - m_lastTime;
        m_lastTime = now;
        return rval;
    }

    // get time expired since the timer was reset
    uint64_t MicroTimer::getTimeFromStart()
    {
        return getMicroTime() - m_startTime;
    }

    // Test if the interval of time has passed since reset
    bool MicroTimer::isExpired()
    {
        return (getTimeFromStart() > m_interval);
    }

    // Class to calculate statistics on Stuff using cheap and easy avg func
    StreamTimeStats::StreamTimeStats()
    {
        clear();
    }

    void StreamTimeStats::clear()
    {
        peak = 0.0;
        mean = 0.0;
        sigma = 0.0;
        windowSize = 100.0;
    }

    void StreamTimeStats::addSample(float sample)
    {
        if (sample > peak)
        {
            peak = sample;
        }
        else
        {
            peak -= 0.05;
        }
        mean += sample / windowSize;
        mean *= (windowSize - 1) / windowSize;
        sigma += std::abs(mean - sample) / windowSize;
        sigma *= (windowSize - 1) / windowSize;
    }

}