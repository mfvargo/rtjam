#ifndef JAM_DIRECTORY_H
#define JAM_DIRECTORY_H

#include "json.hpp"
#include <curl/curl.h>

using json = nlohmann::json;

namespace JamNetStuff
{

  class JamDirectory
  {
  public:
    JamDirectory();
    void loadFromNetwork();
    std::string findUser(uint id);
    void printOut();

  private:
    json directory;
    std::map<uint, std::string> users;
  };

}

#endif