#ifndef LEVEL_METERS_STATE_INCLUDED
#define LEVEL_METERS_STATE_INCLUDED

#include "../common/jamrtime.h"

struct RTJamState {
  float channelLevels[MIX_CHANNELS];
  float bufferDepths[MIX_CHANNELS];
  float masterLevel;
  float inputLeft;
  float inputRight;

  RTJamState() {
    for (int i=0; i<MIX_CHANNELS; i++) {
      channelLevels[i] = -60.0f;
    }
    masterLevel = -60.0f;
  }

  void levelUpdate(float* channels, float* depths) {
    for (int i=0; i<MIX_CHANNELS; i++) {
      channelLevels[i] = channels[i];
      bufferDepths[i] = depths[i];
    }
  }
};


#endif // LEVEL_METERS_STATE_INCLUDED