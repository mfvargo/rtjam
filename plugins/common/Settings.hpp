#ifndef JAMRTIME_SETTINGS_HPP_INCLUDED
#define JAMRTIME_SETTINGS_HPP_INCLUDED

#include "DistrhoPlugin.hpp"
#include "json.hpp"
using json = nlohmann::json;

START_NAMESPACE_DISTRHO

class Settings {
  public:
      Settings();
      int getOrSetValue(const char* key, int value);
      void setValue(const char* key, int value);
      void saveToFile();
      void loadFromFile();

  protected:
    json defValues;

  private:
};

END_NAMESPACE_DISTRHO

#endif