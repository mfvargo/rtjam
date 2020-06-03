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

    uint64_t getMicroTime(){
        uint64_t rval;
        struct timeval currentTime;
        gettimeofday(&currentTime, NULL);
        rval = (currentTime.tv_sec - THE_BEGINNING_OF_TIME) * (int)1e6;
        rval += currentTime.tv_usec;
        return rval;
    }

    // Class to get microsecond time Stamps
    MicroTimer::MicroTimer() {
        lastTime = 0;
    }
    uint64_t MicroTimer::getExpiredTime() {
        uint64_t now = getMicroTime();
        if (lastTime == 0) {
            startTime = now;
            lastTime = now;
        }
        uint64_t rval = now - lastTime;
        lastTime = now;
        return rval;
    }

    uint64_t MicroTimer::getTimeFromStart() {
        return getMicroTime() - startTime;
    }


    // Class to calculate statistics on Stuff using cheap and easy avg func
    StreamTimeStats::StreamTimeStats() {
        clear();
    }

    void StreamTimeStats::clear() {
        mean = 0.0;
        dxdt = 0.0;
        sigma = 0.0;
        sigmaPrime = 0.0;
        windowSize = 100.0;
    }
        
    void StreamTimeStats::addSample(float sample) {
        mean += sample/windowSize;
        mean *= (windowSize - 1) / windowSize;
        dxdt += (mean - sample)/windowSize;
        dxdt *= (windowSize - 1) / windowSize;
        sigma += std::abs(mean - sample)/windowSize;
        sigma *= (windowSize - 1) / windowSize;
        sigmaPrime += std::abs(sigma - std::abs(mean - sample))/windowSize;
        sigmaPrime *= (windowSize - 1) / windowSize;
    }

}