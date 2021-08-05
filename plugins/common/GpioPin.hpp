#pragma once

#include <gpiod.h>
#include <iostream>

using namespace std;

class GpioPin
{
public:
  static void InitializeGpioChip()
  {
    s_chip = gpiod_chip_open_by_name("gpiochip0");
    if (s_chip == NULL)
    {
      cerr << "error opening gpiochip0" << endl;
    }
  }
  void init(int pinNumber, string name)
  {
    m_pin = pinNumber;
    m_line = gpiod_chip_get_line(s_chip, m_pin);
    if (m_line == NULL)
    {
      cerr << "error opening line: " << m_pin << endl;
    }
    if (gpiod_line_request_output(m_line, name.c_str(), 0) == -1)
    {
      cerr << "error setting line to output: " << m_pin << endl;
    }
  };
  void set(bool highLow)
  {
    if (m_line == NULL)
    {
      cerr << "you cannot set a line that was not initialized!" << endl;
      return;
    }
    // Set the pin
    gpiod_line_set_value(m_line, highLow);
  };

  static gpiod_chip *s_chip;

private:
  int m_pin;
  struct gpiod_line *m_line = NULL;
};