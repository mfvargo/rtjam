#include "JamDirectory.hpp"
#include <iostream>

namespace JamNetStuff {

  std::string readBuffer;

  size_t callback(void *data, size_t size, size_t nmemb, void*) {
    readBuffer.append((char*) data, size * nmemb);
    return size * nmemb;
  }

  JamDirectory::JamDirectory() {
    CURL *curl = curl_easy_init();
    if(curl) {
      CURLcode res;
      curl_easy_setopt(curl, CURLOPT_URL, "http://music.basscleftech.com/users/index.json");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
      res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);
      if (res == CURLE_OK) {
        directory = json::parse(readBuffer);
        for (auto& el : directory["users"]) {
          users.insert(std::make_pair(el["id"], el["name"]));
        }
      }
    }
  }

  std::string JamDirectory::findUser(uint id) {
    std::map<uint, std::string>::iterator it = users.find(id);
    if ( it != users.end()) {
      return it->second;
    } else {
      return std::to_string(id);
    }
  }

}