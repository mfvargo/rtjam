#pragma once

#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class Settings
{
public:
  Settings();
  int getOrSetValue(const char *key, int value);
  std::string getOrSetValue(const char *key, std::string value);
  void setValue(const char *key, int value);
  void setValue(const char *key, std::string value);
  void saveToFile();
  void loadFromFile();
  void saveVersionFile();

protected:
  json defValues;
  string m_filename;

private:
};
