#ifndef JAMRTIME_SETTINGS_HPP_INCLUDED
#define JAMRTIME_SETTINGS_HPP_INCLUDED

#include "json.hpp"
using json = nlohmann::json;

class Settings {
  public:
      Settings();
      int getOrSetValue(const char* key, int value);
      std::string getOrSetValue(const char* key, std::string value);
      void setValue(const char* key, int value);
      void setValue(const char* key, std::string value);
      void saveToFile();
      void loadFromFile();

  protected:
    json defValues;

  private:
};

#endif