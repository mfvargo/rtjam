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
#include "../common/NotoSans_Regular.ttf.hpp"
// #include "Window.hpp"
START_NAMESPACE_DISTRHO

namespace Art = RTJamArt;

// -----------------------------------------------------------------------
// Init / Deinit

UIRTJam::UIRTJam()
   : UI(Art::background_2Width, Art::background_2Height),
      fImgBackground(Art::background_2Data, Art::background_2Width, Art::background_2Height, GL_BGR),
      fSlideLine(Art::slidelineData, Art::slidelineWidth, Art::slidelineHeight, GL_BGR)
{
    // Read envrionment
    clickOn = getenv("CLICK_ON") != NULL;
    
    // level sliders
    Image sliderImage(Art::smallSliderData, Art::smallSliderWidth, Art::smallSliderHeight);
    Point<int> sliderPosStart(20, 180);
    Point<int> sliderPosEnd(20, 350);

    float mixerLow = -30.0f;
    float mixerHigh = 12.0f;
    // Input 0
    fVol[0] = new ImageSlider(this, sliderImage);
    fVol[0]->setId(PluginRTJam::paramChanGain1);
    fVol[0]->setInverted(true);
    fVol[0]->setStartPos(sliderPosStart);
    fVol[0]->setEndPos(sliderPosEnd);
    fVol[0]->setRange(mixerLow, mixerHigh);
    fVol[0]->setCallback(this);

    sliderPosStart.setX(sliderPosStart.getX() + 100);
    sliderPosEnd.setX(sliderPosEnd.getX() + 100);
    // Input 1
    fVol[1] = new ImageSlider(this, sliderImage);
    fVol[1]->setId(PluginRTJam::paramChanGain2);
    fVol[1]->setInverted(true);
    fVol[1]->setStartPos(sliderPosStart);
    fVol[1]->setEndPos(sliderPosEnd);
    fVol[1]->setRange(mixerLow, mixerHigh);
    fVol[1]->setCallback(this);

    // Now for the channels
    Corners[0].setPos(237, 25);
    Corners[1].setPos(417, 25);
    Corners[2].setPos(597, 25);
    Corners[3].setPos(237, 225);
    Corners[4].setPos(417, 225);
    Corners[5].setPos(597, 225);
    int spacing = 54;
    for (int i=1; i<MAX_JAMMERS; i++) {
        sliderPosStart.setPos(Corners[i-1]);
        sliderPosEnd.setPos(sliderPosStart.getX(), sliderPosStart.getY() + 120);

        // Input 0
        sliderPosStart.setX(sliderPosStart.getX() + spacing-7);
        sliderPosEnd.setX(sliderPosEnd.getX() + spacing - 7);
        fVol[i*2] = new ImageSlider(this, sliderImage);
        fVol[i*2]->setId(PluginRTJam::paramChanGain1 + i*2);
        fVol[i*2]->setInverted(true);
        fVol[i*2]->setStartPos(sliderPosStart);
        fVol[i*2]->setEndPos(sliderPosEnd);
        fVol[i*2]->setRange(mixerLow, mixerHigh);
        fVol[i*2]->setCallback(this);
        // Input 1
        sliderPosStart.setX(sliderPosStart.getX() + spacing);
        sliderPosEnd.setX(sliderPosEnd.getX() + spacing);
        fVol[i*2+1] = new ImageSlider(this, sliderImage);
        fVol[i*2+1]->setId(PluginRTJam::paramChanGain1 + i*2+1);
        fVol[i*2+1]->setInverted(true);
        fVol[i*2+1]->setStartPos(sliderPosStart);
        fVol[i*2+1]->setEndPos(sliderPosEnd);
        fVol[i*2+1]->setRange(mixerLow, mixerHigh);
        fVol[i*2+1]->setCallback(this);
    }
    // Room selectors
    for (int i=0; i<MAX_ROOMS; i++) {
        fRooms[i] = new ImageSwitch(this,
                        Image(Art::room_offData, Art::room_offWidth, Art::room_offHeight, GL_BGR),
                        Image(Art::room_onData, Art::room_onWidth, Art::room_onHeight, GL_BGR));
        fRooms[i]->setId(PluginRTJam::paramRoom0 + i);
        fRooms[i]->setAbsolutePos(20, 20 + i * 30);
        fRooms[i]->setCallback(this);
    }
    fRooms[0]->setDown(true);
    // Master Volume
    fSliderMaster = new ImageSlider(this, sliderImage);
    fSliderMaster->setId(PluginRTJam::paramMasterVol);
    fSliderMaster->setInverted(true);
    fSliderMaster->setStartPos(158, 3);
    fSliderMaster->setEndPos(158, 88);
    fSliderMaster->setRange(mixerLow, mixerHigh);
    fSliderMaster->setCallback(this);

    // set default values
    programLoaded(0);

    loadSharedResources();
    NanoVG::FontId notoSansId = createFontFromMemory("noto_sans", (const uchar *)font_notosans::notosans_ttf, font_notosans::notosans_ttf_size, 0);
    printf("NototFont ID: %d\n", notoSansId);
    NanoVG::FontId dejaVuSansId = findFont(NANOVG_DEJAVU_SANS_TTF);
    printf("DejaFont ID: %d\n", dejaVuSansId);

    // LabelBoxes
    // for(int i=0; i<2; i++) {
    //     labels[i] = new LabelBox(this, Size<uint>(400, 400));
    //     labels[i]->setText("Bobby");
    //     labels[i]->setFontId(notoSansId);
    //     labels[i]->setFontSize(36.0f);
    //     // labels[i]->setAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    //     // labels[i]->setMargin(Margin(0, 0, labels[i]->getHeight() / 2.0f, 0));
    //     labels[i]->setAbsolutePos(i*100, 20); 
    //     labels[i]->setVisible(true);
    // }

}

UIRTJam::~UIRTJam() {
    // Delete the sliders
    /*
    for (int i=0; i<MIX_CHANNELS; i++) {
        delete fVol[i];
    }
    for (int i=0; i<MAX_ROOMS; i++) {
        delete fRooms[i];
    }
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
void UIRTJam::parameterChanged(uint32_t index, float value) {
    return;
    switch (index)
    {
      case PluginRTJam::paramChanGain1:
      case PluginRTJam::paramChanGain2:
      case PluginRTJam::paramChanGain3:
      case PluginRTJam::paramChanGain4:
      case PluginRTJam::paramChanGain5:
      case PluginRTJam::paramChanGain6:
      case PluginRTJam::paramChanGain7:
      case PluginRTJam::paramChanGain8:
      case PluginRTJam::paramChanGain9:
      case PluginRTJam::paramChanGain10:
      case PluginRTJam::paramChanGain11:
      case PluginRTJam::paramChanGain12:
      case PluginRTJam::paramChanGain13:
      case PluginRTJam::paramChanGain14:
          fVol[index - PluginRTJam::paramChanGain1]->setValue(value);
          break;
      case PluginRTJam::paramMasterVol:
          fSliderMaster->setValue(value);
          break;
    }
}

/**
  A program has been loaded on the plugin side.
  This is called by the host to inform the UI about program changes.
*/
void UIRTJam::programLoaded(uint32_t index) {
    printf("program Load %d\n", index);
    if (index != 0)
        return;

    // Default values
    fVol[0]-> setValue(6.0f);
    fVol[1]-> setValue(6.0f);
    for (int i=2; i<MIX_CHANNELS; i++) {
        fVol[i]->setValue(0.0f);
    }
}

/**
  Optional callback to inform the UI about a sample rate change on the plugin side.
*/
void UIRTJam::sampleRateChanged(double newSampleRate) {
    (void)newSampleRate;
}

// -----------------------------------------------------------------------
// Optional UI callbacks

/**
  Idle callback.
  This function is called at regular intervals.
*/
void UIRTJam::uiIdle() {
    repaint();
    if (PluginRTJam* const dspPtr = (PluginRTJam*)getPluginInstancePointer())
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


/**
  A function called to draw the view contents with NanoVG.
*/
void UIRTJam::onNanoDisplay() {
}

void UIRTJam::onDisplay() {
    fImgBackground.draw();

    Point<int> drawPos(40, 20);
    float yScale = 0.7f;

    // // Master meter (post fade)
    // drawPos.setPos(60, 15);
    // fMeterBar.drawAt(drawPos, 100, 1.0 - ((fState.masterLevel + 60)/60));
    // drawPos.setX(drawPos.getX() + 40);
    // fSlideLine.xScale = 1.0f;
    // fSlideLine.yScale = yScale;
    // fSlideLine.drawAt(drawPos);

    // Input section
    drawPos.setPos(10, 180);
    // Input level 0
    fMeterBar.drawAt(drawPos, 200, 1.0 - (fState.inputLeft + 66)/60);
    drawPos.setX(drawPos.getX() + 32);
    // Slider line
    fSlideLine.xScale = 1.0f;
    fSlideLine.yScale = 1.0f;
    fSlideLine.drawAt(drawPos);
    // Output level 0
    drawPos.setX(drawPos.getX() + 24);
    fMeterBar.drawAt(drawPos, 200, 1.0 - ((fState.channelLevels[0] + 60)/60));

    // Input level 1
    drawPos.setX(111);
    fMeterBar.drawAt(drawPos, 200, 1.0 - (fState.inputRight + 66)/60);
    // Slider line
    drawPos.setX(drawPos.getX() + 32);
    fSlideLine.xScale = 1.0f;
    fSlideLine.yScale = 1.0f;
    fSlideLine.drawAt(drawPos);
    // Output level 1
    drawPos.setX(drawPos.getX() + 24);
    fMeterBar.drawAt(drawPos, 200, 1.0 - ((fState.channelLevels[1] + 60)/60));

    // Master Volume section
    drawPos.setPos(150, 10);
    // output level
    fMeterBar.drawAt(drawPos, 100, 1.0 - (fState.masterLevel + 60)/60);
    drawPos.setX(drawPos.getX() + 32);
    // Slider line
    fSlideLine.xScale = 1.0f;
    fSlideLine.yScale = 0.5f;
    fSlideLine.drawAt(drawPos);

    // Channel meters post fader
    const int height = 140;
    const int spacing = 27;
    for(int i=1; i<MAX_JAMMERS; i++) {
        drawPos.setPos(Corners[i-1]);
        // Smoother
        const float depth = fState.clientIds[i] == EMPTY_SLOT ? 0.0 : fState.bufferDepths[i*2];
        fMeterBar.drawAt(drawPos, height, 1.0 - depth);
        drawPos.setX(drawPos.getX() + spacing);
        // fSlideLine.xScale = 1.0f;
        // fSlideLine.yScale = yScale;
        // fSlideLine.drawAt(drawPos);

        // Input 0
        drawPos.setX(drawPos.getX() + spacing-10);
        fMeterBar.drawAt(drawPos, height, 1.0 - ((fState.channelLevels[i*2] + 60)/60));
        drawPos.setX(drawPos.getX() + spacing);
        fSlideLine.xScale = 1.0f;
        fSlideLine.yScale = yScale;
        fSlideLine.drawAt(drawPos);

        // Input 1
        drawPos.setX(drawPos.getX() + spacing);
        fMeterBar.drawAt(drawPos, height, 1.0 - ((fState.channelLevels[i*2+1] + 60)/60));
        drawPos.setX(drawPos.getX() + spacing);
        fSlideLine.xScale = 1.0f;
        fSlideLine.yScale = yScale;
        fSlideLine.drawAt(drawPos);
    }
    
    // LabelBoxes
    // for(int i=0; i<1; i++) {
    //     labels[i]->drawMe();
    // }

    // printf("5001: %s\n", jamDirectory.findUser(5001).c_str());
    // if (clickOn) {
    //     // Metronome
    //     drawPos.setPos(50, 115);
    //     for(int i=0; i<4; i++) {
    //         fMeterBar.drawAt(drawPos, 30, 1.0 - (fState.beat == i));
    //         drawPos.setX(drawPos.getX() + spacing);
    //     }
    // }
}


// -----------------------------------------------------------------------
// Optional widget callbacks; return true to stop event propagation, false otherwise.

/**
  A function called when a key is pressed or released.
*/
bool UIRTJam::onKeyboard(const KeyboardEvent& ev) {
    return false;
    (void)ev;
}

/**
  A function called when a special key is pressed or released.
*/
bool UIRTJam::onSpecial(const SpecialEvent& ev) {
    return false;
    (void)ev;
}

/**
  A function called when a mouse button is pressed or released.
*/
bool UIRTJam::onMouse(const MouseEvent& ev) {
    return false;
    (void)ev;
}

/**
  A function called when the mouse pointer moves.
*/
bool UIRTJam::onMotion(const MotionEvent& ev) {
    return false;
    (void)ev;
}

/**
  A function called on scrolling (e.g. mouse wheel or track pad).
*/
bool UIRTJam::onScroll(const ScrollEvent& ev) {
    return false;
    (void)ev;
}

// -----------------------------------------------------------------------
// Widget Callbacks

void UIRTJam::imageButtonClicked(ImageButton*, int)
{
    // printf("button push\n");
    // if (button != fButtonAbout)
    //     return;

    // fAboutWindow.exec();
}

void UIRTJam::imageSwitchClicked(ImageSwitch* button, bool down) {
    // Radio button logic for rooms
    switch(button->getId()) {
        case PluginRTJam::paramRoom0:
        case PluginRTJam::paramRoom1:
        case PluginRTJam::paramRoom2:
            if (down) {
                for (int i=0; i<MAX_ROOMS; i++) {
                    fRooms[i]->setDown(false);
                }
                button->setDown(true);
                setParameterValue(button->getId(), down);
                programLoaded(0);
            } else {
                // Existing button going up, ignore it
                button->setDown(true);
            }

        break;
        case PluginRTJam::paramInputMonitor:
            setParameterValue(button->getId(), down);
        break;
    }
}

void UIRTJam::imageSliderDragStarted(ImageSlider* slider)
{
    editParameter(slider->getId(), true);
}

void UIRTJam::imageSliderDragFinished(ImageSlider* slider)
{
    editParameter(slider->getId(), false);
}

void UIRTJam::imageSliderValueChanged(ImageSlider* slider, float value)
{
    setParameterValue(slider->getId(), value);
}

// -----------------------------------------------------------------------

UI* createUI() {
    return new UIRTJam();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
