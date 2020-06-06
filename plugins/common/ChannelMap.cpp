/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 */

#include "../common/JamNetStuff.hpp"
#include <time.h>

#define EMPTY_SLOT 40000

namespace JamNetStuff {

  ChannelMap::ChannelMap() {
    for (int i=0; i<MAX_JAMMERS; i++) {
      channels[i].clientId = EMPTY_SLOT;   // Illegal value, max is 32k
      channels[i].KeepAlive = 0;
    }
  }

  void ChannelMap::setMyId(uint32_t id) {
    myId = id;
    channels[0].clientId = id;
  }

  void ChannelMap::clear() {
    for (int i=1; i<MAX_JAMMERS; i++) {
      channels[i].clientId = EMPTY_SLOT;
    }
  }

  int ChannelMap::getChannel(uint32_t clientId) {
    // dumpOut();
    time_t now = time(NULL);
    // Clear out the dead wood
    pruneStaleChannels(now);
    // Find us if we are there!
    for (int i=0; i<MAX_JAMMERS; i++) {
      if (channels[i].clientId == clientId) {
        channels[i].KeepAlive = now;
        return i;
      }
    }
    // If we got here, we don't know this dude.  put him in an open slot
    for (int i=0; i<MAX_JAMMERS; i++) {
      if (channels[i].clientId == EMPTY_SLOT) {
        channels[i].clientId = clientId;
        channels[i].KeepAlive = now;
        return i;
      }
    }
    return -1;
  }

  void ChannelMap::pruneStaleChannels(time_t now) {
    // Never clear out slot 0 cause that's the local dude
    for (int i=1; i<MAX_JAMMERS; i++) {
      if ((now - channels[i].KeepAlive) > EXPIRATION_IN_SECONDS) {
        channels[i].clientId = EMPTY_SLOT;
      }
    }
  }

  void ChannelMap::dumpOut() {
    for (int i=0; i<MAX_JAMMERS; i++) {
      printf("chan: %d, clientId: %d, ", i, channels[i].clientId);
    }
    printf("\n");
  }
}