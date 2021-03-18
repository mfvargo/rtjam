#ifndef LEVELDATA_RTJAM_HPP
#define LEVELDATA_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "SharedMemory.hpp"


struct RTJamLevels {
  float channelLevels[MIX_CHANNELS];
  float bufferDepths[MIX_CHANNELS];
  uint32_t clientIds[MAX_JAMMERS];
  float masterLevel;
  float inputLeft;
  float inputRight;
  char beat;
};

class LevelData {
  public:
     LevelData();
    ~LevelData();

    void lock();
    void unlock();

    RTJamLevels* m_pJamLevels;

  private:
    CSharedMemory m_sharedMemory;
};

#endif
