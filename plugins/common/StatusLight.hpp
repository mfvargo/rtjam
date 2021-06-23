#pragma once

#include <wiringPi.h>

class StatusLight
{
public:
  enum StatusColor
  {
    black = 0,
    green,
    orange,
    red
  };

  enum StatusFunction
  {
    inputOne = 0,
    inputTwo,
    status
  };

  static void startInit()
  {
    wiringPiSetup();
  };

  void init(StatusFunction func);
  void set(float power);
  void set(StatusColor color);
  void flash(int duration);

private:
  int m_redPin;
  int m_greenPin;
  int m_flashDuration;
  bool m_bFlash;

  void flashThreadFunc();
};