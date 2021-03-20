#pragma once

#include "restincurl.h"
#include "json.hpp"

using json = nlohmann::json;

class RTJamNationApi {
  public:
    void testMe(std::string url);

  private:
    json m_resultBody;
};

