/*
 * RTJam audio effect based on DISTRHO Plugin Framework (DPF)
 *
 * SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2020 Mike Vargo <mfvargo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "PluginRTJam.hpp"
#include "levelMeters.hpp"

static const float kAMP_DB = 8.656170245f;

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

PluginRTJam::PluginRTJam()
    : Plugin(paramCount, presetCount, 0),  // paramCount param(s), presetCount program(s), 0 states
      fState(nullptr)
{
    // set window size on input power bars
    leftInput.windowSize = 30.0;
    rightInput.windowSize = 30.0;

    // set localMonitor Off
    monitorInput = false;

    // Initialize the jamSocket
    settings.loadFromFile();
    // settings.setValue("port", 7891);
    int port = settings.getOrSetValue("port", 7891);
    jamSocket.initClient(port);
    
    // set default values
    loadProgram(0);

    // reset
    deactivate();
}

PluginRTJam::~PluginRTJam() {
    DISTRHO_SAFE_ASSERT(fState == nullptr);
}

// -----------------------------------------------------------------------
// Init

void PluginRTJam::initParameter(uint32_t index, Parameter& parameter) {
    if (index >= paramCount)
        return;

    switch (index) {
        case paramChanOneGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 1";
            parameter.symbol     = "one";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanTwoGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 2";
            parameter.symbol     = "two";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanThreeGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 3";
            parameter.symbol     = "three";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanFourGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 4";
            parameter.symbol     = "four";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanFiveGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 5";
            parameter.symbol     = "five";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanSixGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 6";
            parameter.symbol     = "six";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanSevenGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 7";
            parameter.symbol     = "seven";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramChanEightGain:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Ch 8";
            parameter.symbol     = "eight";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
        case paramMasterVol:
            parameter.hints      = kParameterIsAutomable;
            parameter.name       = "Master";
            parameter.symbol     = "master";
            parameter.unit       = "dB";
            parameter.ranges.def = 0.0f;
            parameter.ranges.min = -60.0f;
            parameter.ranges.max = 12.0f;
            break;
    }
}

/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void PluginRTJam::initProgramName(uint32_t index, String& programName) {
    if (index != 0)
        return;
    programName = "Default";
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void PluginRTJam::sampleRateChanged(double newSampleRate) {
    fSampleRate = newSampleRate;
}

/**
  Get the current value of a parameter.
*/
float PluginRTJam::getParameterValue(uint32_t index) const {
    switch (index) {
        case paramChanOneGain:
        case paramChanTwoGain:
        case paramChanThreeGain:
        case paramChanFourGain:
        case paramChanFiveGain:
        case paramChanSixGain:
        case paramChanSevenGain:
        case paramChanEightGain:
            return jamMixer.gains[index - paramChanOneGain];
        case paramMasterVol:
            return jamMixer.masterVol;
        default:
            // All the float values are good for 0.0
            return 0.0f;
    }
}

/**
  Change a parameter value.
*/
void PluginRTJam::setParameterValue(uint32_t index, float value) {
    switch (index) {
        case paramChanOneGain:
        case paramChanTwoGain:
        case paramChanThreeGain:
        case paramChanFourGain:
        case paramChanFiveGain:
        case paramChanSixGain:
        case paramChanSevenGain:
        case paramChanEightGain:
            jamMixer.gains[index - paramChanOneGain] = dbToFloat(value);
            break;
        case paramMasterVol:
            jamMixer.masterVol = dbToFloat(value);
            break;
        case paramSmooth1:
        case paramSmooth2:
        case paramSmooth3:
        case paramSmooth4:
            jamMixer.setBufferSmoothness(index - paramSmooth1, value);
            break;
        case paramInputMonitor:
            monitorInput = (value > 0.5f);
            break;
    }
}

float PluginRTJam::dbToFloat(float value) {
    if (value < -59.5) {
        return 0.0f;
    }
    return std::exp( (value/72.0f) * 72.0f / kAMP_DB);
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void PluginRTJam::loadProgram(uint32_t index) {
    if (index != 0)
        return;

    // reset the frameCount
    frameCount = 0;

    // reset filter values
    activate();
}

// -----------------------------------------------------------------------
// Process

void PluginRTJam::activate() {
    // plugin is activated
    jamSocket.isActivated = true;
}

void PluginRTJam::deactivate() {
    settings.saveToFile();
    jamSocket.isActivated = false;
}


void PluginRTJam::run(const float** inputs, float** outputs,
                      uint32_t frames) {

    // Get input levels
    float leftPow = 0.0;
    float rightPow = 0.0;
    for (uint32_t i=0; i<frames; i++) {
        leftPow += pow(inputs[0][i], 2);
        rightPow += pow(inputs[1][i], 2);
    }
    leftPow /= frames + 1;
    if (leftPow > 1E-6) {
        leftPow = 10 * log10(leftPow);
    } else {
        leftPow = -60.0f;
    }
    rightPow /= frames + 1;
    if (rightPow > 1E-6) {
        rightPow = 10 * log10(rightPow);
    } else {
        rightPow = -60.0f;
    }
    leftInput.addSample(leftPow);
    rightInput.addSample(rightPow);

    // Local monitoring
    jamMixer.addLocalMonitor(inputs, frames);

    // Do the network thingy..
    jamSocket.sendPacket(inputs, frames);
    jamSocket.readPackets(&jamMixer);

    // Feed the output from the mixer
    jamMixer.getMix(outputs, frames);
    // if (++frameCount%2000 == 0) {
    //     jamMixer.dumpOut();
    // }

    // Update data to be shared with the U/X
    const MutexLocker csm(fMutex);
    if (fState != nullptr) {
        fState->levelUpdate(jamMixer.channelLevels, jamMixer.bufferDepths);
        fState->masterLevel = jamMixer.masterLevel;
        fState->inputLeft = leftInput.mean;
        fState->inputRight = rightInput.mean;
    }

    if (monitorInput) {
        memcpy(outputs[0], inputs[0], sizeof(float) * frames);
        memcpy(outputs[1], inputs[1], sizeof(float) * frames);
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin() {
    return new PluginRTJam();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
