#pragma once

#include "restincurl.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class RTJamNationApi {
  public:
    RTJamNationApi(string urlbase);
    void testMe(string url);
    bool jamUnitPing(string token);

  private:
    json m_resultBody;
    string m_urlBase;
};

