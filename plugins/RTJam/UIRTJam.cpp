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
#include "UIRTJam.hpp"
#include "NotoSans_Regular.ttf.hpp"
// #include "Window.hpp"

START_NAMESPACE_DISTRHO

namespace Art = RTJamArt;

// -----------------------------------------------------------------------
// Init / Deinit

UIRTJam::UIRTJam()
    : UI(Art::background_2Width, Art::background_2Height),
      fImgBackground(Art::background_2Data, Art::background_2Width, Art::background_2Height, GL_BGR)
{
    fNanoText.loadSharedResources();
    fNanoFont = fNanoText.findFont(NANOVG_DEJAVU_SANS_TTF);
    Image sliderImage(Art::smallSliderData, Art::smallSliderWidth, Art::smallSliderHeight);

    Point<int> sliderPosStart(20, 180);
    Point<int> sliderPosEnd(20, 330);

    // Now for the channels
    Corners[0].setPos(237, 25);
    Corners[1].setPos(417, 25);
    Corners[2].setPos(597, 25);
    Corners[3].setPos(237, 225);
    Corners[4].setPos(417, 225);
    Corners[5].setPos(597, 225);
    // Room selectors
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        fRooms[i] = new ImageSwitch(this,
                                    Image(Art::room_offData, Art::room_offWidth, Art::room_offHeight, GL_BGR),
                                    Image(Art::room_onData, Art::room_onWidth, Art::room_onHeight, GL_BGR));
        fRooms[i]->setId(PluginRTJam::paramRoom0 + i);
        fRooms[i]->setAbsolutePos(20, 20 + i * 30);
        fRooms[i]->setCallback(this);
    }
    fRooms[0]->setDown(true);

    // set default values
    programLoaded(0);
}

UIRTJam::~UIRTJam()
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        delete fRooms[i];
    }

    // delete fReverb;
    /*
    // This is some threadsafe way to null a pointer in the DSP module
    if (PluginRTJam* const dspPtr = (PluginRTJam*)getPluginInstancePointer())
    {
        const MutexLocker csm(dspPtr->fMutex);
        dspPtr->fState = nullptr;
    }
    */
}
// -----------------------------------------------------------------------
// DSP/Plugin callbacks

/**
  A parameter has changed on the plugin side.
  This is called by the host to inform the UI about parameter changes.
*/
void UIRTJam::parameterChanged(uint32_t, float)
{
    return;
}

/**
  A program has been loaded on the plugin side.
  This is called by the host to inform the UI about program changes.
*/
void UIRTJam::programLoaded(uint32_t index)
{
    printf("program Load %d\n", index);
    if (index != 0)
        return;
}

/**
  Optional callback to inform the UI about a sample rate change on the plugin side.
*/
void UIRTJam::sampleRateChanged(double newSampleRate)
{
    (void)newSampleRate;
}

// -----------------------------------------------------------------------
// Optional UI callbacks

/**
  Idle callback.
  This function is called at regular intervals.
*/
void UIRTJam::uiIdle()
{
    repaint();
    if (PluginRTJam *const dspPtr = (PluginRTJam *)getPluginInstancePointer())
    {
        // So if the dsp plugin instance does not have my state, let them have it.  Then they can update it.
        if (dspPtr->fState != nullptr)
            return;

        const MutexLocker csm(dspPtr->fMutex);
        dspPtr->fState = &fState;
    }
}

// -----------------------------------------------------------------------
// Widget callbacks

void UIRTJam::onDisplay()
{
    fImgBackground.draw();

    Point<int> drawPos(40, 20);

    // Input section
    drawPos.setPos(10, 180);
    // My Name
    drawText(20, 140, std::to_string(fState.clientIds[0]).c_str());

    // Input level 0
    fMeterBar.drawAt(drawPos, 170, 1.0 - (fState.inputLeft + 66) / 60);
    drawPos.setX(drawPos.getX() + 32);

    // Input level 1
    drawPos.setX(111);
    fMeterBar.drawAt(drawPos, 170, 1.0 - (fState.inputRight + 66) / 60);

    // Channel meters post fader
    for (int i = 1; i < MAX_JAMMERS; i++)
    {
        drawChannel(i);
    }
}

void UIRTJam::drawChannel(int chan)
{
    Point<int> drawPos(Corners[chan - 1]);
    const int height = 140;
    const int spacing = 27;
    char strBuf[32 + 1];
    strBuf[32] = '\0';

    // If nobody is there, then don't draw them
    if (fState.clientIds[chan] == EMPTY_SLOT)
    {
        return;
    }

    // Smooth indicator
    const float depth = fState.clientIds[chan] == EMPTY_SLOT ? 0.0 : fState.bufferDepths[chan * 2];
    fMeterBar.drawAt(drawPos, height, 1.0 - depth);
    std::snprintf(strBuf, 32, "%0.0f", depth * 40);
    drawText(drawPos.getX() - 40, drawPos.getY() + height + 5, strBuf);

    drawPos.setX(drawPos.getX() + spacing);

    drawPos.setX(drawPos.getX() + spacing - 10);
    fMeterBar.drawAt(drawPos, height, 1.0 - ((fState.channelLevels[chan * 2] + 60) / 60));
    drawPos.setX(drawPos.getX() + spacing);

    // Input 1 section
    drawPos.setX(drawPos.getX() + 4 + spacing);
    fMeterBar.drawAt(drawPos, height, 1.0 - ((fState.channelLevels[chan * 2 + 1] + 60) / 60));
    drawPos.setX(drawPos.getX() + spacing);

    // Name for the jammer
    if (fState.clientIds[chan] != EMPTY_SLOT)
    {
        drawPos = Corners[chan - 1];
        drawText(drawPos.getX(), drawPos.getY() - 18, std::to_string(fState.clientIds[chan]).c_str());
    }
}

void UIRTJam::drawText(int x, int y, const char *strBuf)
{
    fNanoText.beginFrame(this);
    fNanoText.fontFaceId(fNanoFont);
    fNanoText.fontSize(16);
    fNanoText.textAlign(NanoVG::ALIGN_CENTER | NanoVG::ALIGN_TOP);
    fNanoText.fillColor(Color(0.0f, 0.0f, 0.0f));
    fNanoText.textBox(x, y, 100.0f, strBuf, nullptr);
    fNanoText.endFrame();
}

// -----------------------------------------------------------------------
// Optional widget callbacks; return true to stop event propagation, false otherwise.
void UIRTJam::imageKnobDragStarted(ImageKnob *knob)
{
    editParameter(knob->getId(), true);
}

void UIRTJam::imageKnobDragFinished(ImageKnob *knob)
{
    editParameter(knob->getId(), false);
}

void UIRTJam::imageKnobValueChanged(ImageKnob *knob, float value)
{
    setParameterValue(knob->getId(), value);
}

/**
  A function called when a key is pressed or released.
*/
bool UIRTJam::onKeyboard(const KeyboardEvent &ev)
{
    return false;
    (void)ev;
}

/**
  A function called when a special key is pressed or released.
*/
bool UIRTJam::onSpecial(const SpecialEvent &ev)
{
    return false;
    (void)ev;
}

/**
  A function called when a mouse button is pressed or released.
*/
bool UIRTJam::onMouse(const MouseEvent &ev)
{
    return false;
    (void)ev;
}

/**
  A function called when the mouse pointer moves.
*/
bool UIRTJam::onMotion(const MotionEvent &ev)
{
    return false;
    (void)ev;
}

/**
  A function called on scrolling (e.g. mouse wheel or track pad).
*/
bool UIRTJam::onScroll(const ScrollEvent &ev)
{
    return false;
    (void)ev;
}

// -----------------------------------------------------------------------
// Widget Callbacks

void UIRTJam::imageButtonClicked(ImageButton *, int)
{
    // printf("button push\n");
    // if (button != fButtonAbout)
    //     return;

    // fAboutWindow.exec();
}

void UIRTJam::imageSwitchClicked(ImageSwitch *button, bool down)
{
    // Radio button logic for rooms
    switch (button->getId())
    {
    case PluginRTJam::paramRoom0:
    case PluginRTJam::paramRoom1:
    case PluginRTJam::paramRoom2:
        if (down)
        {
            for (int i = 0; i < MAX_ROOMS; i++)
            {
                fRooms[i]->setDown(false);
            }
            button->setDown(true);
            setParameterValue(button->getId(), down);
            programLoaded(0);
        }
        else
        {
            // Existing button going up, ignore it
            button->setDown(true);
        }

        break;
    case PluginRTJam::paramInputMonitor:
        setParameterValue(button->getId(), down);
        break;
    }
}

void UIRTJam::imageSliderDragStarted(ImageSlider *slider)
{
    editParameter(slider->getId(), true);
}

void UIRTJam::imageSliderDragFinished(ImageSlider *slider)
{
    editParameter(slider->getId(), false);
}

void UIRTJam::imageSliderValueChanged(ImageSlider *slider, float value)
{
    setParameterValue(slider->getId(), value);
}

// -----------------------------------------------------------------------

UI *createUI()
{
    return new UIRTJam();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
