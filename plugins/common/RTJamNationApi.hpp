#pragma once

#define RESTINCURL_ENABLE_ASYNC 0

#ifndef GIT_HASH
#define GIT_HASH "No Githash in Makefile"
#endif

#include "restincurl.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class RTJamNationApi
{
public:
  RTJamNationApi(string urlbase);
  bool status();
  bool jamUnitDeviceRegister();
  bool jamUnitPing(string token, bool connected);
  bool broadcastUnitDeviceRegister();
  bool broadcastUnitPing(string token);
  bool activateRoom(string token, string name, int port);
  bool playerList(string roomToken);
  json m_resultBody;
  long m_httpResponseCode;
  bool checkLinkStatus(string interface);

private:
  bool put(string url, json body);
  bool get(string url, json body);
  bool post(string url, json body);
  string m_urlBase;
  string m_lanIp;
  string m_macAddress;
};
