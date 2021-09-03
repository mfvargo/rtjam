#include "LabelBox.hpp"
// #include "Mathf.hpp"

#include "NotoSans_Regular.ttf.hpp"

START_NAMESPACE_DISTRHO

LabelBox::LabelBox(NanoWidget *widget, Size<uint> size) noexcept : NanoWidget(widget)
{
    setSize(size);

    //using namespace WOLF_FONTS;
    //createFontFromMemory("chivo_bold", (const uchar *)chivo_bold, chivo_bold_size, 0);
}

void LabelBox::onNanoDisplay()
{
    drawMe();
}

void LabelBox::drawMe()
{
    const float width = getWidth();
    const float height = getHeight();
    const float verticalMargin = 6.0f;
    const float boxOutlineWidth = 2.0f;

    //Box background
    beginPath();

    fillColor(Color(34, 34, 34, 255));
    strokeColor(Color(64, 64, 64, 255));
    strokeWidth(boxOutlineWidth);

    rect(0, 0, width, height);
    fill();
    stroke();

    closePath();

    //Shadow at top of box
    beginPath();

    strokeColor(0, 0, 0, 255);
    strokeWidth(boxOutlineWidth);

    moveTo(boxOutlineWidth, boxOutlineWidth);
    lineTo(width - boxOutlineWidth, boxOutlineWidth);
    stroke();

    closePath();

    //Text
    beginPath();

    // fontFace("chivo_bold");
    fontFace(NANOVG_DEJAVU_SANS_TTF);
    fontSize(16.0f);
    fillColor(Color(255, 255, 255, 255));
    textAlign(ALIGN_CENTER | ALIGN_MIDDLE);

    text(std::round(width / 2.0f), std::round(height / 2.0f + verticalMargin / 2.0f - 2), fText, NULL);

    closePath();
}

void LabelBox::setFontSize(float fontSize)
{
    fFontSize = fontSize;
}

float LabelBox::getFontSize()
{
    return fFontSize;
}

void LabelBox::setText(const char *text)
{
    fText = text;
}

const char *LabelBox::getText()
{
    return fText;
}

void LabelBox::setFontId(NanoVG::FontId fontId)
{
    fFontId = fontId;
}

NanoVG::FontId LabelBox::getFontId()
{
    return fFontId;
}

END_NAMESPACE_DISTRHO