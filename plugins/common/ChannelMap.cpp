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
#include <string.h>


namespace JamNetStuff {

  ChannelMap::ChannelMap() {
    clear();
  }

  void ChannelMap::setMyId(uint32_t id) {
    myId = id;
    channels[0].clientId = id;
  }

  void ChannelMap::clear() {
    for (int i=0; i<MAX_JAMMERS; i++) {
      channels[i].clientId = EMPTY_SLOT;   // Illegal value, max is 32k
      channels[i].KeepAlive = time(NULL);
      memset(&channels[i].Address, '\0', sizeof channels[i].Address);
    }
  }

  void ChannelMap::getClientAddr(int idx, sockaddr_in *addr) { 
    memcpy(addr, &channels[idx].Address, sizeof channels[idx].Address); 
  };

  int ChannelMap::getChannel(uint32_t clientId, sockaddr_in *addr) {
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
        if (addr != NULL) {
          memcpy(&channels[i].Address, addr, sizeof channels[i].Address);
        }
        printf("adding %d: ", i);
        dumpOut();
        return i;
      }
    }
    return -1;
  }

  int ChannelMap::getChannelBySenderIp(uint32_t clientId, sockaddr_in *addr) {
    // dumpOut();
    time_t now = time(NULL);
    // Clear out the dead wood
    pruneStaleChannels(now);
    // Find us if we are there!
    for (int i=0; i<MAX_JAMMERS; i++) {
      if (channels[i].Address.sin_addr.s_addr == addr->sin_addr.s_addr && 
          channels[i].Address.sin_port == addr->sin_port) {
        channels[i].KeepAlive = now;
        return i;
      }
    }
    // If we got here, we don't know this dude.  put him in an open slot
    for (int i=0; i<MAX_JAMMERS; i++) {
      if (channels[i].clientId == EMPTY_SLOT) {
        channels[i].clientId = clientId;
        channels[i].KeepAlive = now;
        if (addr != NULL) {
          memcpy(&channels[i].Address, addr, sizeof channels[i].Address);
        }
        printf("adding %d: ", i);
        dumpOut();
        return i;
      }
    }
    return -1;
  }

  void ChannelMap::pruneStaleChannels(time_t now, int startAt) {
    // Never clear out slot 0 cause that's the local dude
    for (int i=startAt; i<MAX_JAMMERS; i++) {
      if (channels[i].clientId != EMPTY_SLOT && (now - channels[i].KeepAlive) > EXPIRATION_IN_SECONDS) {
        printf("nuking %d: ", i);
        dumpOut();
        channels[i].clientId = EMPTY_SLOT;
        memset(&channels[i].Address, '\0', sizeof channels[i].Address);
      }
    }
  }

  void ChannelMap::dumpOut() {
    char ipString[24];
    for (int i=0; i<MAX_JAMMERS; i++) {
      makeIpString(channels[i].clientId, ipString);
      printf("%s  ", ipString);
    }
    printf("\n");
  }

  void ChannelMap::makeIpString(unsigned long s_addr, char* ipString) {
    unsigned char octet[4]  = {0,0,0,0};
    for (int i=0; i<4; i++)
    {
        octet[i] = ( s_addr >> (i*8) ) & 0xFF;
    }
    sprintf(ipString, "%03d.%03d.%03d.%03d", octet[0],octet[1],octet[2],octet[3]);
  }

}