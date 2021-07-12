#pragma once

#include "json.hpp"
#include <map>
#include "EffectSetting.hpp"

using json = nlohmann::json;

class Effect
{
public:
  // Base class implemention.  Classes that derive from Effect will call this function first.  Then set up their
  // own settings that they can receive.
  virtual void init()
  {
    EffectSetting bypass;
    bypass.init(
        "bypass",                   // Name
        EffectSetting::booleanType, // Type of setting
        0,                          // Min value
        1,                          // Max value
        1,                          // Step Size
        EffectSetting::footswitch);
    bypass.setBoolValue(true);
    m_settingMap.insert(std::pair<std::string, EffectSetting>(bypass.name(), bypass));
  };
  json getSettings()
  {
    json rval = {{"name", m_name}, {"settings", json::object()}};
    for (std::map<std::string, EffectSetting>::iterator it = m_settingMap.begin(); it != m_settingMap.end(); ++it)
    {
      rval["settings"][it->first] = it->second.toJson();
    }
    return rval;
  }
  bool setSettingValue(json setting)
  {
    std::map<std::string, EffectSetting>::iterator it = m_settingMap.find(setting["name"]);
    if (it == m_settingMap.end())
    {
      // No setting found with that name!
      return false;
    }
    // set the setting
    if (!it->second.setFromJson(setting))
    {
      // This means the setting was out of range
      return false;
    }
    loadFromConfig();
    return true;
  }

  // Effects must implement.  They should load their variables from the setting map
  virtual void loadFromConfig()
  {
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