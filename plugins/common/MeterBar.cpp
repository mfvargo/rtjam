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

#include "MeterBar.hpp"
#include "RTJamArt.hpp"

namespace Art = RTJamArt;

namespace JamNetStuff {

  JamMeterBar::JamMeterBar()
    : fLitUp(Art::meter_onData, Art::meter_onWidth, Art::meter_onHeight, GL_BGR),
      fDark(Art::meter_offData, Art::meter_offWidth, Art::meter_offHeight, GL_BGR)
  {
    // Build the bad boy here
  }

  void JamMeterBar::drawAt(Point<int> pos, int height, float scale) {
    if (scale < 0.0) {
      scale = 0.0;
    }
    if (scale > 1.0) {
      scale = 1.0;
    }
    float baseYScale = (1.0 * height) / fLitUp.getHeight();
    if (baseYScale > 1.0f) {
      baseYScale = 1.0f;
    }
    fLitUp.xScale = 1.0f;
    fLitUp.yScale = baseYScale;
    fLitUp.drawAt(pos);
    fDark.xScale = 1.0f;
    fDark.yScale = baseYScale * scale;
    fDark.drawAt(pos);
  }

}