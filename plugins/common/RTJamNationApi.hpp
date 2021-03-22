#pragma once

#define RESTINCURL_ENABLE_ASYNC 0

#include "restincurl.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class RTJamNationApi {
  public:
    RTJamNationApi(string urlbase);
    bool status();
    bool jamUnitPing(string token);
    bool broadcastUnitPing(string token);
    bool activateRoom(string token, string name, int port);

  private:
    bool put(string url, json body);
    bool get(string url, json body);
    bool post(string url, json body);
    json m_resultBody;
    string m_urlBase;
};

