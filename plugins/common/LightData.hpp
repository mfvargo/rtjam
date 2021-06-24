#pragma once

#include "SharedMemory.hpp"

enum LightColors
{
  black = 0,
  green,
  orange,
  red
};

struct LightSettings
{
  LightColors status;
  LightColors inputOne;
  LightColors inputTwo;
};

class LightData
{
public:
  LightData();
  ~LightData();

  LightSettings *m_pLightSettings;

private:
  CSharedMemory m_sharedMemory;
};
