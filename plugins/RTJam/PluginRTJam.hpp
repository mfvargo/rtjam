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

#ifndef PLUGIN_RTJAM_H
#define PLUGIN_RTJAM_H

#include "DistrhoPlugin.hpp"
#include "extra/Mutex.hpp"
#include "JamNetStuff.hpp"
#include "JamDirectory.hpp"
#include "Settings.hpp"

class UIRTJam;
struct RTJamState;

START_NAMESPACE_DISTRHO

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(v, min, max) (MIN((max), MAX((min), (v))))
#endif

#ifndef DB_CO
#define DB_CO(g) ((g) > -90.0f ? powf(10.0f, (g)*0.05f) : 0.0f)
#endif

// -----------------------------------------------------------------------

class PluginRTJam : public Plugin
{
public:
    enum Parameters
    {
        paramChanGain1 = 0,
        paramChanGain2,
        paramChanGain3,
        paramChanGain4,
        paramChanGain5,
        paramChanGain6,
        paramChanGain7,
        paramChanGain8,
        paramChanGain9,
        paramChanGain10,
        paramChanGain11,
        paramChanGain12,
        paramChanGain13,
        paramChanGain14,
        paramMasterVol,
        paramInputMonitor,
        paramRoom0,
        paramRoom1,
        paramRoom2,
        paramCount
    };

    PluginRTJam();

    ~PluginRTJam();

protected:
    // Room switch
    void switchRoom(int roomParam);

    // -------------------------------------------------------------------
    // Information

    const char *getLabel() const noexcept override
    {
        return "RTJam";
    }

    const char *getDescription() const override
    {
        return "Real Time Music Jam";
    }

    const char *getMaker() const noexcept override
    {
        return "basscleftech.com";
    }

    const char *getHomePage() const override
    {
        return "http://music.basscleftech.com/";
    }

    const char *getLicense() const noexcept override
    {
        return "https://spdx.org/licenses/MIT";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(0, 1, 0);
    }

    // Go to:
    //
    // http://service.steinberg.de/databases/plugin.nsf/plugIn
    //
    // Get a proper plugin UID and fill it in here!
    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('b', 'c', 'j', 'r');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter &parameter) override;
    void initProgramName(uint32_t index, String &programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;
    void loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // Optional

    // Optional callback to inform the plugin about a sample rate change.
    void sampleRateChanged(double newSampleRate) override;

    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void deactivate() override;
    void run(const float **, float **outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:
    double fSampleRate;

    Settings settings;
    JamNetStuff::JamMixer jamMixer;
    JamNetStuff::JamSocket jamSocket;
    JamNetStuff::StreamTimeStats leftInput;
    JamNetStuff::StreamTimeStats rightInput;

    bool monitorInput;

    float dbToLinear(float value);
    int frameCount;

    bool reverbOnInputOne;
    float *reverbBuf[2];

    Mutex fMutex;
    RTJamState *fState;
    friend class UIRTJam;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginRTJam)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // #ifndef PLUGIN_RTJAM_H
