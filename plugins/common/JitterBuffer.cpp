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

#define MIN_DEPTH 512
#define MAX_DEPTH 4096
#define MIN_SIGMA 10.0

namespace JamNetStuff
{

    JitterBuffer::JitterBuffer() {
        maxDepth = JITTER_SAMPLES - 512;
        flush();
        bufferStats.windowSize = 50;
    }

    void JitterBuffer::flush() {
        readIdx = 0;
        writeIdx = 0;
        numOverruns = 0;
        numUnderruns = 0;
        numPuts = 0;
        numGets = 0;
        numDropped = 0;
        lastSequence = 0;
        targetDepth = MIN_DEPTH;
        nSigma = MIN_SIGMA;
    }

    void JitterBuffer::setSmoothness(float smooth) {
        flush();
        nSigma = MIN_SIGMA + (smooth * 100.0);
        // targetDepth = MIN_DEPTH + (smooth * 9600);  //a 0.1 smooth adds 20msec
    }

    int JitterBuffer::depth() {
        if (writeIdx >= readIdx) {
            // Distance between the indexes
            return writeIdx - readIdx;
        }
        // Distance to the end of the buffer + whats on the front
        return JITTER_SAMPLES - readIdx + writeIdx;
    }

    float JitterBuffer::getAvgDepth() {
        return (targetDepth * 1.0) / MAX_DEPTH;
        // return bufferStats.mean / targetDepth;
    }

    void JitterBuffer::putIn(const float* buffer, int frames, uint32_t seqNo) {
        int dropped = lastSequence - seqNo;
        lastSequence = seqNo;
        numPuts++;
        if (dropped < -1) {
            // lost packets
            numDropped++;
        }
        if (depth() > maxDepth) {
            // TODO:  This is the wrong approach.  Should keep the last one and push the first one off.
            numOverruns++;
            return;
        }
        if (JITTER_SAMPLES - writeIdx >= frames) {
            // there is room on the end
            copySamples(&myBuffer[writeIdx], buffer, frames);
            writeIdx += frames;
        } else {
            // Need to write some on the end and some on the front (wrap)
            copySamples(&myBuffer[writeIdx], buffer, JITTER_SAMPLES - writeIdx);
            copySamples(&myBuffer[0], &buffer[JITTER_SAMPLES - writeIdx], frames - (JITTER_SAMPLES - writeIdx));
            writeIdx = frames - (JITTER_SAMPLES - writeIdx);
        }
    }

    void JitterBuffer::getOut(float* buffer, int frames) {
        numGets++;
        bufferStats.addSample(depth());
        // dynamic target depth
        unsigned desiredDepth = nSigma * bufferStats.sigma;
        if (desiredDepth > targetDepth) {
            // make the buffer longer
            if (targetDepth < MAX_DEPTH) {
                targetDepth += 1;
            }
        } else {
            if (numGets%4 == 0 && targetDepth > MIN_DEPTH) {
                targetDepth -= 1;
            }
        }
        if (depth() < frames) {
            // Not enough for a frame
            if (numPuts > 0) {
                numUnderruns++;
            }
            // Play back the last frame
            memcpy(buffer, lastFrame, frames * sizeof(float));
            // fade the last frame
            for(int i = 0; i< frames; i++) {
                lastFrame[i] *= 0.85;
            }
            return;
        }
        int framesLeft = JITTER_SAMPLES - readIdx;
        int numOverBuf = depth() - targetDepth;
        if (numOverBuf > 0) {
            // Advance the read pointer to flush old data
            if (framesLeft >= numOverBuf) {
                readIdx += numOverBuf;
            } else {
                readIdx = numOverBuf - framesLeft;
            }
        }
        framesLeft = JITTER_SAMPLES - readIdx;
        if (framesLeft >= frames) {
            // There is room on the end to just pull it off
            copySamples(buffer, &myBuffer[readIdx], frames);
            readIdx += frames;
        } else {
            // need to read some off the end and some from the head.
            copySamples(buffer, &myBuffer[readIdx], framesLeft);
            copySamples(&buffer[framesLeft], &myBuffer[0], frames - framesLeft);
            readIdx = frames - framesLeft;
        }
        // Save the lastFrame in case we starve
        memcpy(lastFrame, buffer, frames * sizeof(float));
    }

    void JitterBuffer::copySamples(float* dst, const float* src, int count) {
        for(int i=0; i<count; i++) {
            *dst++ = *src++;
        }
    }

    void JitterBuffer::dumpOut() {
        printf(
            "avgDepth: %08.1f\t target: %06d\t under: %05d\7 delta_u:%03.2f\t seq: %d\n",
            bufferStats.mean,
            targetDepth,
            numUnderruns,
            bufferStats.sigma,
            lastSequence
        );
    }

}