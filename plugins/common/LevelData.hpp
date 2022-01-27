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
  float roomLevelLeft;
  float roomPeakLeft;
  float roomLevelRight;
  float roomPeakRight;
  float inputLeftFreq;
  float inputRightFreq;
  bool leftTunerOn;
  bool rightTunerOn;
  bool leftRoomMute;
  bool rightRoomMute;
  uint64_t jsonTimeStamp;
};

struct JsonInfo
{
  char buffer[64 * 1024];
};

struct MidiRingBuffer
{
  unsigned readIdx;
  unsigned writeIdx;
  unsigned char ringBuffer[32 * 28]; // 32 3 byte midi events
  char deviceName[128];
};

class LevelData
{
public:
  LevelData() : m_sharedMemory("rtjamValues")
  {
    m_sharedMemory.Create(sizeof(RTJamLevels) + sizeof(JsonInfo) + sizeof(MidiRingBuffer));
    m_sharedMemory.Attach();
    m_pJamLevels = (RTJamLevels *)m_sharedMemory.GetData();
    m_pJsonInfo = (char *)m_sharedMemory.GetData() + sizeof(RTJamLevels);
    m_pRingBuffer = (MidiRingBuffer *)((char *)m_sharedMemory.GetData() + sizeof(RTJamLevels) + sizeof(JsonInfo));
    m_pRingBuffer->readIdx = 0;
    m_pRingBuffer->writeIdx = 0;
  }
  ~LevelData()
  {
    m_sharedMemory.Detach();
  }

  RTJamLevels *m_pJamLevels;
  char *m_pJsonInfo;
  MidiRingBuffer *m_pRingBuffer;

private:
  CSharedMemory m_sharedMemory;
  RTJamLevels m_levels;
  JsonInfo m_json;
};
