#include "Settings.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pwd.h>

  Settings::Settings() {
    // do some constructing
    defValues = {
    };
    // find the home directory for settings.json
    char *homedir = getenv("HOME");
    if (homedir != NULL) {
      m_filename = homedir;
    } else {
      uid_t uid = getuid();
      struct passwd *pw = getpwuid(uid);
      if (pw != NULL) {
        m_filename = pw->pw_dir;
      }
    }
    m_filename += "/settings.json";
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
    std::ofstream outFile(m_filename);
    outFile << defValues;
    // Open a file and save the settings
  }

  void Settings::loadFromFile() {
    // Load settings from a file
    std::ifstream infile(m_filename);
    if (infile.is_open()) {
      infile >> defValues;
    }
  }
