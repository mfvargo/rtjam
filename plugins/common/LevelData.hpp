#pragma once

#include "JamNetStuff.hpp"
#include "SharedMemory.hpp"

struct RTJamLevels
{
  float channelLevels[MIX_CHANNELS];
  float bufferDepths[MIX_CHANNELS];
  uint32_t clientIds[MAX_JAMMERS];
  float masterLevel;
  float inputLeft;
  float inputRight;
  char beat;
  bool isConnected;
  float peakLevels[MIX_CHANNELS];
  float peakLeft;
  float peakRight;
  float peakMaster;
};

struct JsonInfo
{
  char buffer[64 * 1024];
};

class LevelData
{
public:
  LevelData() : m_sharedMemory("rtjamValues")
  {
    m_sharedMemory.Create(sizeof(RTJamLevels) + sizeof(JsonInfo));
    m_sharedMemory.Attach();
    m_pJamLevels = (RTJamLevels *)m_sharedMemory.GetData();
    m_pJsonInfo = (char *)m_sharedMemory.GetData() + sizeof(RTJamLevels);
  }
  ~LevelData()
  {
    m_sharedMemory.Detach();
  }

  RTJamLevels *m_pJamLevels;
  char *m_pJsonInfo;

private:
  CSharedMemory m_sharedMemory;
  RTJamLevels m_levels;
  JsonInfo m_json;
};
