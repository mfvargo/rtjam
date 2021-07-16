#pragma once

#include "JamNetStuff.hpp"

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
  LevelData()
  {
    m_pJamLevels = &m_levels;
    m_pJsonInfo = m_json.buffer;
  }

  void lock();
  void unlock();

  RTJamLevels *m_pJamLevels;
  char *m_pJsonInfo;

private:
  RTJamLevels m_levels;
  JsonInfo m_json;
};
