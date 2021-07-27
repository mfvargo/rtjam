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
    JamMixer::JamMixer()
    {
        // Initialize stuff here
        conversionBuf[0] = cBuf0;
        conversionBuf[1] = cBuf1;
        reset();
    }

    // reset to default values
    void JamMixer::reset()
    {
        for (int i = 0; i < MIX_CHANNELS; i++)
        {
            gains[i] = 1.0;
            channelLevels[i] = 0;
            bufferDepths[i] = 0.0;
            levelStats[i].windowSize = 30.0;
            jitterBuffers[i].flush();
        }
        masterVol = 1.0;
        masterLevel = 0.0f;
        masterPeak = 0.0f;
        masterStats.windowSize = 30.0;
    }

    /* print out some stats */
    void JamMixer::dumpOut()
    {
        for (int i = 0; i < MIX_CHANNELS; i++)
        {
            printf("Chan: %d\t", i);
            jitterBuffers[i].dumpOut();
        }
        printf("\n");
    }
    /* get some data for the output */
    void JamMixer::getMix(float **outputs, uint32_t frames)
    {
        float levelSums[MIX_CHANNELS];

        // get a frame out of each mix buffer
        for (uint32_t i = 0; i < MIX_CHANNELS; i++)
        {
            jitterBuffers[i].getOut(mixBuffers[i], frames);
            levelSums[i] = 0.0f;
        }
        masterLevel = 0.0f;
        // sum all the buffers.
        for (uint32_t i = 0; i < frames; i++)
        {
            float sum = 0.0;
            for (int j = 0; j < MIX_CHANNELS; j++)
            {
                outputs[2 + j][i] = mixBuffers[j][i];
                levelSums[j] += pow(gains[j] * mixBuffers[j][i], 2);
                sum += masterVol * gains[j] * mixBuffers[j][i];
            }
            if (sum > 1.0)
                sum = 1.0;
            if (sum < -1.0)
                sum = -1.0;
            masterLevel += pow(sum, 2);
            outputs[0][i] = sum;
            outputs[1][i] = sum;
        }
        for (int i = 0; i < MIX_CHANNELS; i++)
        {
            levelSums[i] /= frames + 1;
            if (levelSums[i] > 1E-6)
            {
                levelSums[i] = 10 * log10(levelSums[i]);
            }
            else
            {
                levelSums[i] = -60.0f;
            }
            levelStats[i].addSample(levelSums[i]);
            channelLevels[i] = levelStats[i].mean;
            peakLevels[i] = levelStats[i].peak;
            bufferDepths[i] = jitterBuffers[i].getAvgDepth();
        }
        masterLevel /= frames + 1;
        if (masterLevel > 1E-6)
        {
            masterLevel = 10 * log10(masterLevel);
        }
        else
        {
            masterLevel = -60.0;
        }
        masterStats.addSample(masterLevel);
        masterLevel = masterStats.mean;
        masterPeak = masterStats.peak;
    }

    /* give the mixer a packet to chew on */
    void JamMixer::addData(JamPacket *packet)
    {
        int samples = packet->decodeJamBuffer(conversionBuf);
        int locChan = packet->getChannel() * 2;
        beat = packet->getBeatCount();
        if (locChan >= 0 && samples > 0)
        {
            jitterBuffers[locChan].putIn(conversionBuf[0], samples, packet->getSequenceNo());
            jitterBuffers[locChan + 1].putIn(conversionBuf[1], samples, packet->getSequenceNo());
        }
    }

    void JamMixer::addLocalMonitor(const float **input, uint32_t frames)
    {
        jitterBuffers[0].putIn(input[0], frames, 1);
        jitterBuffers[1].putIn(input[1], frames, 1);
    }

    void JamMixer::setBufferSmoothness(int channel, float smooth)
    {
        jitterBuffers[2 * channel].setSmoothness(smooth);
        jitterBuffers[2 * channel + 1].setSmoothness(smooth);
    }
}