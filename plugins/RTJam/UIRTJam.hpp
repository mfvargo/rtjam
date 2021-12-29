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

#ifndef UI_RTJAM_H
#define UI_RTJAM_H

#include "DistrhoUI.hpp"
#include "NanoVG.hpp"
#include "ImageWidgets.hpp"
#include "PartialImage.hpp"
#include "RTJamArt.hpp"
#include "levelMeters.hpp"
#include "MeterBar.hpp"
#include "JamDirectory.hpp"
// #include "LabelBox.hpp"

#define MAX_ROOMS 3

using DGL::NanoVG;

START_NAMESPACE_DISTRHO

class UIRTJam : public UI,
                public ImageButton::Callback,
                public ImageSlider::Callback,
                public ImageSwitch::Callback,
                public ImageKnob::Callback
{
public:
    UIRTJam();
    ~UIRTJam() override;

protected:
    void parameterChanged(uint32_t, float value) override;
    void programLoaded(uint32_t index) override;
    void sampleRateChanged(double newSampleRate) override;

    void uiIdle() override;

    // -------------------------------------------------------------------
    // Widget Callbacks

    void imageButtonClicked(ImageButton *button, int) override;
    void imageSwitchClicked(ImageSwitch *button, bool down) override;
    void imageSliderDragStarted(ImageSlider *slider) override;
    void imageSliderDragFinished(ImageSlider *slider) override;
    void imageSliderValueChanged(ImageSlider *slider, float value) override;
    void imageKnobDragStarted(ImageKnob *knob) override;
    void imageKnobDragFinished(ImageKnob *knob) override;
    void imageKnobValueChanged(ImageKnob *knob, float value) override;

    // void onNanoDisplay() override;
    void onDisplay() override;

    bool onKeyboard(const KeyboardEvent &ev) override;
    bool onSpecial(const SpecialEvent &ev) override;
    bool onMouse(const MouseEvent &ev) override;
    bool onMotion(const MotionEvent &ev) override;
    bool onScroll(const ScrollEvent &ev) override;

    // Draw a channels u/x
    void drawChannel(int channel);
    void drawText(int x, int y, const char *strBuf);

private:
    Image fImgBackground;
    PartialImage fSlideLine;
    Image fSmoothButtonNormal, fsmoothButtonPressed;
    ImageSlider *fVol[MIX_CHANNELS];
    ImageSwitch *fRooms[MAX_ROOMS];
    ImageSwitch *fReverb;
    JamNetStuff::JamMeterBar fMeterBar;
    ScopedPointer<ImageSlider> fSliderMaster;
    ScopedPointer<ImageSwitch> fMonitorInputButton;
    RTJamState fState;
    Point<int> Corners[MAX_JAMMERS - 1];

    NanoVG fNanoText;
    NanoVG::FontId fNanoFont;

    JamNetStuff::JamDirectory jamDirectory;
    std::vector<ImageKnob *> fKnobs;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIRTJam)
};

END_NAMESPACE_DISTRHO

#endif
