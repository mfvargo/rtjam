#pragma once

#include "json.hpp"
#include <map>
#include "EffectSetting.hpp"
#include <iostream>

using json = nlohmann::json;

using namespace std;

class Effect
{
public:
  // Base class implemention.  Classes that derive from Effect will call this function first.  Then set up their
  // own settings that they can receive.
  virtual void init()
  {
    // all effects have a bypass setting they can receive.
    EffectSetting bypass;
    bypass.init(
        "bypass",                   // Name
        EffectSetting::booleanType, // Type of setting
        0,                          // Min value
        1,                          // Max value
        1,                          // Step Size
        EffectSetting::footswitch);
    bypass.setBoolValue(true);
    addSetting(bypass);
  };

  void addSetting(EffectSetting setting)
  {
    setting.setIndex(m_settingMap.size());
    m_settingMap.insert(std::pair<std::string, EffectSetting>(setting.name(), setting));
  };

  // This function will return all the settings this effect has as json.
  json getSettings()
  {
    json rval = {{"name", m_name}, {"settings", json::array()}};
    for (std::map<std::string, EffectSetting>::iterator it = m_settingMap.begin(); it != m_settingMap.end(); ++it)
    {
      rval["settings"].push_back(it->second.toJson());
    }
    return rval;
  }
  // This function will set the value of a specific setting given some json
  // Json must be in the form {"name":"settingName", "value": xxx }
  bool setSettingValue(json setting)
  {
    // find the setting with this name.
    std::map<std::string, EffectSetting>::iterator it = m_settingMap.find(setting["name"]);
    if (it == m_settingMap.end())
    {
      // No setting found with that name!
      cerr << setting["name"] << " Setting not found" << endl;
      return false;
    }
    // set the value on the found EffectSetting.  This will check min/max values etc.
    if (!it->second.setFromJson(setting))
    {
      cerr << setting["value"] << " Value out of range" << endl;
      return false;
    }
    // Now reload the parameters for the effect from the settings.
    loadFromConfig();
    return true;
  }

  // Effects must implement.  They should load their variables from the setting map
  virtual void loadFromConfig()
  {
    // load my bypass setting
    std::map<std::string, EffectSetting>::iterator it = m_settingMap.find("bypass");
    if (it != m_settingMap.end())
    {
      m_byPass = it->second.getBoolValue();
    }
  };

  void doProcess(const float *input, float *output, int framesize)
  {
    // check for bypass
    if (getByPass())
    {
      return byPass(input, output, framesize);
    }
    return process(input, output, framesize);
  }
  // void setParam();
  virtual void process(const float *input, float *output, int framesize) = 0;

  bool getByPass()
  {
    return m_byPass;
  }

protected:
  std::string m_name;
  std::map<std::string, EffectSetting> m_settingMap;

private:
  bool m_byPass = false;

  // Code to bypass this effect
  void byPass(const float *input, float *output, int framesize)
  {
    for (int i = 0; i < framesize; i++)
    {
      output[i] = input[i];
    }
  }
};