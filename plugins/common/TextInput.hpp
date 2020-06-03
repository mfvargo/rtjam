#ifndef TEXT_INPUT_HPP_INCLUDED
#define TEXT_INPUT_HPP_INCLUDED

#include "Widget.hpp"
#include "NanoVG.hpp"
#include <string>

START_NAMESPACE_DISTRHO

class TextInput : public NanoWidget
{
public:
    explicit TextInput(Widget *parent, NanoVG* pNanoText);

protected:
    void onNanoDisplay() override;
    bool onKeyboard(const KeyboardEvent &ev) override;
    NanoVG* fNanoText;

private:
    std::string Label;
    DISTRHO_LEAK_DETECTOR(TextInput)
};

END_NAMESPACE_DISTRHO
#endif
