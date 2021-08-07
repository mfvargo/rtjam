#pragma once

#include "LightData.hpp"
#include "GpioPin.hpp"

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
    GpioPin::InitializeGpioChip();
  };

  void init(StatusFunction func);
  void set(float power);
  void set(LightColors color);

private:
  GpioPin m_redPin;
  GpioPin m_greenPin;
};