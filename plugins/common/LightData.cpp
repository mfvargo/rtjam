#include "LightData.hpp"
#include <iostream>

LightData::LightData() : m_sharedMemory("rtjamLightSettings")
{
  m_sharedMemory.Create(sizeof(LightSettings));
  m_sharedMemory.Attach();
  m_pLightSettings = (LightSettings *)m_sharedMemory.GetData();
}

LightData::~LightData()
{
  m_sharedMemory.Detach();
}
