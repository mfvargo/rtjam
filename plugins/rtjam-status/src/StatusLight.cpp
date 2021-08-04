#include "StatusLight.hpp"

struct gpiod_chip *GpioPin::s_chip;

void StatusLight::init(StatusLight::StatusFunction func)
{
  // see https://pinout.xyz/pinout/wiringpi for mappings
  switch (func)
  {
  case inputOne:
    m_redPin.init(6, "inputOneRed");
    m_greenPin.init(5, "inputOneGreen");
    break;
  case inputTwo:
    m_redPin.init(8, "inputTwoRed");
    m_greenPin.init(7, "inputTwoGreen");
    break;
  case status:
    m_redPin.init(23, "statusRed");
    m_greenPin.init(24, "statusGreen");
    break;
  }
}

void StatusLight::set(LightColors color)
{
  switch (color)
  {
  case black:
    m_redPin.set(false);
    m_greenPin.set(false);
    break;
  case green:
    m_redPin.set(false);
    m_greenPin.set(true);
    break;
  case orange:
    m_redPin.set(true);
    m_greenPin.set(true);
    break;
  case red:
    m_redPin.set(true);
    m_greenPin.set(false);
    break;
  }
}

void StatusLight::set(float power)
{
  if (power < -45.0)
  {
    set(black);
  }
  else if (power < -30)
  {
    set(green);
  }
  else if (power < -20)
  {
    set(orange);
  }
  else
  {
    set(red);
  }
}
