#include "Settings.hpp"
#include <iostream>
#include <fstream>

START_NAMESPACE_DISTRHO

  Settings::Settings() {
    // do some constructing
    defValues = {
    };
  }

  int Settings::getOrSetValue(const char* key, int value) {
    if (defValues[key].is_null()) {
      setValue(key, value);
    }
    return defValues[key];
  }

  std::string Settings::getOrSetValue(const char* key, std::string value) {
    if (defValues[key].is_null()) {
      setValue(key, value);
    }
    return defValues[key];

  }
  void Settings::setValue(const char* key, int value) {
    defValues[key] = value;
  }
  void Settings::setValue(const char* key, std::string value) {
    defValues[key] = value;
  }

  void Settings::saveToFile() {
    std::ofstream outFile("settings.json");
    outFile << defValues;
    // Open a file and save the settings
  }

  void Settings::loadFromFile() {
    // Load settings from a file
    std::ifstream infile("settings.json");
    if (infile.is_open()) {
      infile >> defValues;
    }
  }


END_NAMESPACE_DISTRHO