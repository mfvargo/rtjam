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
    : Plugin(paramCount, 0, 0), // paramCount param(s), presetCount program(s), 0 states
      fState(nullptr)
{
    // set window size on input power bars
    leftInput.windowSize = 30.0;
    rightInput.windowSize = 30.0;

    // set localMonitor Off
    monitorInput = false;

    // Initialize the jamSocket
    switchRoom(paramRoom0);

    // set default values
    loadProgram(0);

    // ReverbToggle
    reverbOnInputOne = true;
}

PluginRTJam::~PluginRTJam()
{
    // DISTRHO_SAFE_ASSERT(fState == nullptr);
}

void PluginRTJam::switchRoom(int roomParam)
{
    // Initialize the jamSocket
    settings.loadFromFile();
    std::string serverName = settings.getOrSetValue("server", std::string(SERVER_NAME));
    char *client_env = getenv("RTJAM_CLIENT");
    uint32_t clientId;
    if (client_env)
    {
        clientId = atoi(client_env);
    }
    else
    {
        clientId = rand() % 32768;
    }
    clientId = settings.getOrSetValue("clientId", clientId);
    settings.saveToFile();
    int port = SERVER_PORT;
    switch (roomParam)
    {
    case paramRoom0:
        port = 7891;
        break;
    case paramRoom1:
        port = 7892;
        break;
    case paramRoom2:
        port = 7893;
        break;
    }
    jamMixer.reset();
    jamMixer.gains[0] = dbToLinear(6.0);
    jamMixer.gains[1] = dbToLinear(6.0);
    for (int i = 0; i < MAX_JAMMERS; i++)
    {
        jamMixer.setBufferSmoothness(i, 0.2);
    }

    jamSocket.initClient(serverName.c_str(), port, clientId);
}
// -----------------------------------------------------------------------
// Init

void PluginRTJam::initParameter(uint32_t index, Parameter &parameter)
{
    if (index >= paramCount)
        return;

    switch (index)
    {
    case paramChanGain1:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 1";
        parameter.symbol = "one";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain2:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 2";
        parameter.symbol = "two";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain3:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 3";
        parameter.symbol = "three";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain4:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 4";
        parameter.symbol = "four";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain5:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 5";
        parameter.symbol = "five";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain6:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 6";
        parameter.symbol = "six";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain7:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 7";
        parameter.symbol = "seven";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain8:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 8";
        parameter.symbol = "eight";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain9:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 9";
        parameter.symbol = "nine";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain10:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 10";
        parameter.symbol = "ten";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain11:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 11";
        parameter.symbol = "eleven";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain12:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 12";
        parameter.symbol = "twelve";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain13:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 13";
        parameter.symbol = "thirteen";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramChanGain14:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Ch 14";
        parameter.symbol = "fourteen";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramMasterVol:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Master";
        parameter.symbol = "master";
        parameter.unit = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -60.0f;
        parameter.ranges.max = 12.0f;
        break;
    case paramReverbMix:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Reverb";
        parameter.symbol = "reverb";
        parameter.unit = "amount";
        parameter.ranges.def = 0.1f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}

/**
  Set the name of the program @a index.
  This function will be called once, shortly after the plugin is created.
*/
void PluginRTJam::initProgramName(uint32_t index, String &programName)
{
    printf("init Program");
    if (index != 0)
        return;
    programName = "Default";
}

// -----------------------------------------------------------------------
// Internal data

/**
  Optional callback to inform the plugin about a sample rate change.
*/
void PluginRTJam::sampleRateChanged(double newSampleRate)
{
    fSampleRate = newSampleRate;
    fVerb.setSampleRate(newSampleRate);
}

/**
  Get the current value of a parameter.
*/
float PluginRTJam::getParameterValue(uint32_t index) const
{
    switch (index)
    {
    case paramChanGain1:
    case paramChanGain2:
    case paramChanGain3:
    case paramChanGain4:
    case paramChanGain5:
    case paramChanGain6:
    case paramChanGain7:
    case paramChanGain8:
    case paramChanGain9:
    case paramChanGain10:
    case paramChanGain11:
    case paramChanGain12:
    case paramChanGain13:
    case paramChanGain14:
        return jamMixer.gains[index - paramChanGain1];
    case paramMasterVol:
        return jamMixer.masterVol;
    case paramRoom0:
        return 1.0f;
    default:
        // All the float values are good for 0.0
        return 0.0f;
    }
}

/**
  Change a parameter value.
*/
void PluginRTJam::setParameterValue(uint32_t index, float value)
{
    printf("param changed %u %f\n", index, value);
    switch (index)
    {
    case paramChanGain1:
    case paramChanGain2:
    case paramChanGain3:
    case paramChanGain4:
    case paramChanGain5:
    case paramChanGain6:
    case paramChanGain7:
    case paramChanGain8:
    case paramChanGain9:
    case paramChanGain10:
    case paramChanGain11:
    case paramChanGain12:
    case paramChanGain13:
    case paramChanGain14:
        if (value < -29.9)
        {
            value = -60.0;
        }
        jamMixer.gains[index - paramChanGain1] = dbToLinear(value);
        break;
    case paramMasterVol:
        jamMixer.masterVol = dbToLinear(value);
        break;
    case paramInputMonitor:
        monitorInput = (value > 0.5f);
        break;
    case paramRoom0:
    case paramRoom1:
    case paramRoom2:
        if (value > 0.5f)
        {
            // Switch to this room
            switchRoom(index);
        }
        break;
    case paramReverbChanOne:
        reverbOnInputOne = (value > 0.5f);
        break;
    case paramReverbMix:
        fVerb.setParameter(MVerb<float>::MIX, value);
        break;
    }
}

float PluginRTJam::dbToLinear(float value)
{
    if (value < -59.5)
    {
        return 0.0f;
    }
    return std::exp((value / 72.0f) * 72.0f / kAMP_DB);
}

/**
  Load a program.
  The host may call this function from any context,
  including realtime processing.
*/
void PluginRTJam::loadProgram(uint32_t index)
{
    printf("loading program %u\n", index);

    fVerb.setParameter(MVerb<float>::DAMPINGFREQ, 0.5f);
    fVerb.setParameter(MVerb<float>::DENSITY, 0.5f);
    fVerb.setParameter(MVerb<float>::BANDWIDTHFREQ, 0.5f);
    fVerb.setParameter(MVerb<float>::DECAY, 0.5f);
    fVerb.setParameter(MVerb<float>::PREDELAY, 0.5f);
    fVerb.setParameter(MVerb<float>::SIZE, 0.75f);
    fVerb.setParameter(MVerb<float>::GAIN, 1.0f);
    fVerb.setParameter(MVerb<float>::MIX, 0.1f);
    fVerb.setParameter(MVerb<float>::EARLYMIX, 0.5f);

    if (index != 0)
        return;

    // reset the frameCount
    frameCount = 0;

    // reset filter values
    activate();
}

// -----------------------------------------------------------------------
// Process

void PluginRTJam::activate()
{
    jamMixer.reset();
    jamMixer.gains[0] = dbToLinear(6.0);
    jamMixer.gains[1] = dbToLinear(6.0);
    jamSocket.isActivated = true;
    fVerb.reset();
}

void PluginRTJam::deactivate()
{
    // settings.saveToFile();
    jamSocket.isActivated = false;
}

void PluginRTJam::run(const float **inputs, float **outputs,
                      uint32_t frames)
{
    // Get input levels
    float leftPow = 0.0;
    float rightPow = 0.0;
    for (uint32_t i = 0; i < frames; i++)
    {
        leftPow += pow(inputs[0][i], 2);
        rightPow += pow(inputs[1][i], 2);
    }
    leftPow /= frames + 1;
    if (leftPow > 1E-6)
    {
        leftPow = 10 * log10(leftPow);
    }
    else
    {
        leftPow = -60.0f;
    }
    rightPow /= frames + 1;
    if (rightPow > 1E-6)
    {
        rightPow = 10 * log10(rightPow);
    }
    else
    {
        rightPow = -60.0f;
    }
    leftInput.addSample(leftPow);
    rightInput.addSample(rightPow);

    // Apply reverb to inputs
    float left[frames];
    float right[frames];
    float *tempOut[2];
    tempOut[0] = left;
    tempOut[1] = right;
    float inLeft[frames];
    float inRight[frames];
    float *tempIn[2];
    tempIn[0] = inLeft;
    tempIn[1] = inRight;
    for (uint32_t i = 0; i < frames; i++)
    {
        // Only give the left input to the reverb engine
        inLeft[i] = inputs[0][i];
        inRight[i] = 0.0f;
    }
    fVerb.process((const float **)tempIn, tempOut, static_cast<int>(frames));
    for (uint32_t i = 0; i < frames; i++)
    {
        if (!reverbOnInputOne)
        {
            tempOut[0][i] = inputs[0][i];
        }
        tempOut[1][i] = inputs[1][i]; // Copy the right channel back in
    }

    // Local monitoring
    jamMixer.addLocalMonitor((const float **)tempOut, frames);

    // Do the network thingy..
    jamSocket.sendPacket((const float **)tempOut, frames);
    jamSocket.readPackets(&jamMixer);

    // Feed the output from the mixer
    jamMixer.getMix(outputs, frames);
    uint32_t ids[MAX_JAMMERS];
    jamSocket.getClientIds(ids);

    // Update data to be shared with the U/X
    const MutexLocker csm(fMutex);
    if (fState != nullptr)
    {
        fState->levelUpdate(jamMixer.channelLevels, jamMixer.bufferDepths);
        fState->masterLevel = jamMixer.masterLevel;
        fState->inputLeft = leftInput.mean;
        fState->inputRight = rightInput.mean;
        fState->beat = jamMixer.getBeat();
        fState->clientIdsUpdate(ids);
    }

    if (monitorInput)
    {
        memcpy(outputs[0], inputs[0], sizeof(float) * frames);
        memcpy(outputs[1], inputs[1], sizeof(float) * frames);
    }
}

// -----------------------------------------------------------------------

Plugin *createPlugin()
{
    return new PluginRTJam();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
