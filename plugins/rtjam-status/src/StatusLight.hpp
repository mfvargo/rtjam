#pragma once

#include <wiringPi.h>
#include "LightData.hpp"

class StatusLight
{
public:
  enum StatusFunction
  {
    inputOne = 0,
    inputTwo,
    status
  };

  static void startInit()
  {
    wiringPiSetupGpio();
  };

  void init(StatusFunction func);
  void set(float power);
  void set(LightColors color);

private:
  int m_redPin;
  int m_greenPin;
  bool m_bFlash;
};