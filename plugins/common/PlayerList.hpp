#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <map>
#include <string>

using namespace std;

  class PlayerList {
    public:
      PlayerList();
      void load(string token);
      int getChannel(unsigned clientId, sockaddr_in *addr = NULL);
      int getChannelBySenderIp(uint32_t clientId, sockaddr_in *addr);
      void dumpOut(bool asIp = true);
      void clear();
      void getClientAddr(int idx, sockaddr_in *addr);
      void pruneStaleChannels(time_t now, int startAt = 1);
      void getClientIds(unsigned* ids);

    private:
      struct Channel {
        uint32_t clientId;
        time_t KeepAlive;
        sockaddr_in Address;
      };
      std::map<unsigned, Channel> m_channelMap;
  };
