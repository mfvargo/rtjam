#include "StatusLight.hpp"

void StatusLight::init(StatusLight::StatusFunction func)
{
  // see https://pinout.xyz/pinout/wiringpi for mappings
  switch (func)
  {
  case inputOne:
    m_redPin = 22;   // GPIO 6
    m_greenPin = 21; // GPIO 5
    break;
  case inputTwo:
    m_redPin = 10;   // GPIO 8
    m_greenPin = 11; // GPIO 7
    break;
  case status:
    m_redPin = 4;   // GPIO 23
    m_greenPin = 5; // GPIO 24
    break;
  }
  pinMode(m_redPin, OUTPUT);
  pinMode(m_greenPin, OUTPUT);
  m_bFlash = false;
}

void StatusLight::set(LightColors color)
{
  switch (color)
  {
  case black:
    digitalWrite(m_redPin, LOW);
    digitalWrite(m_greenPin, LOW);
    break;
  case green:
    digitalWrite(m_redPin, LOW);
    digitalWrite(m_greenPin, HIGH);
    break;
  case orange:
    digitalWrite(m_redPin, HIGH);
    digitalWrite(m_greenPin, HIGH);
    break;
  case red:
    digitalWrite(m_redPin, HIGH);
    digitalWrite(m_greenPin, LOW);
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
