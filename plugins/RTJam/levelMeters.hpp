#ifndef LEVEL_METERS_STATE_INCLUDED
#define LEVEL_METERS_STATE_INCLUDED

#include "../common/jamrtime.h"

struct RTJamState {
  float channelLevels[MIX_CHANNELS];
  float bufferDepths[MIX_CHANNELS];
  uint32_t clientIds[MAX_JAMMERS];
  float masterLevel;
  float inputLeft;
  float inputRight;
  char beat;

  RTJamState() {
    for (int i=0; i<MIX_CHANNELS; i++) {
      channelLevels[i] = -60.0f;
    }
    masterLevel = -60.0f;
    inputLeft = 6.0;
    inputRight = 6.0;
    beat = 0;
  }

  void clientIdsUpdate(uint32_t* ids) {
    for (int i=0; i<MAX_JAMMERS; i++) {
      clientIds[i] = ids[i];
    }
  }
  
  void levelUpdate(float* channels, float* depths) {
    for (int i=0; i<MIX_CHANNELS; i++) {
      channelLevels[i] = channels[i];
      bufferDepths[i] = depths[i];
    }
  }
};


#endif // LEVEL_METERS_STATE_INCLUDED