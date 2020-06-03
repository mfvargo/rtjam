#include "TextInput.hpp"

START_NAMESPACE_DISTRHO

TextInput::TextInput(Widget *widget, NanoVG* pNanoText)
    : NanoWidget(widget->getParentWindow()),
      Label("")

{
  fNanoText = pNanoText;
}

void TextInput::onNanoDisplay()
{
  if (Label.length() > 0) {
    // print some text
    fNanoText->beginFrame ( this );
    fNanoText->fontSize ( 16 );
    fNanoText->textLineHeight(100);
    fNanoText->textAlign (ALIGN_LEFT|ALIGN_TOP);
    fNanoText->fillColor ( Color ( 255, 255, 255 ) );
    fNanoText->text ( 1, 1, Label.c_str(), nullptr );
    fNanoText->endFrame();
  }
}

bool TextInput::onKeyboard(const KeyboardEvent &ev)
{
    uint k = ev.key;
    if (ev.press){
      printf("key %c pressed\n", char(k));
      Label += char(k);
      printf("label = %s\n",Label.c_str());
      repaint();
    }
    return true;
}
END_NAMESPACE_DISTRHO