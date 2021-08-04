#pragma once

#include "json.hpp"
#include "SignalBlock.hpp"
#include <vector>
using json = nlohmann::json;

class EffectSetting
{
public:
  enum Types
  {
    floatType,
    intType,
    booleanType,
  };

  enum Units
  {
    msec,
    dB,
    linear,
    selector,
    footswitch,
  };

  std::string name()
  {
    return m_name;
  }

  // This is how the effect will specify the setting (bottom up)
  void init(std::string name, Types type, float min, float max, float step, Units units)
  {
    m_name = name;
    m_type = type;
    m_min = min;
    m_max = max;
    m_step = step,
    m_units = units;
    m_index = 0;
  };

  void setIndex(int idx)
  {
    m_index = idx;
  };

  void setLabels(std::vector<std::string> labels)
  {
    m_labels = labels;
  };

  // This is how the effect will provide it's setting (bottom up)
  json toJson()
  {
    return {
        {"name", m_name},
        {"index", m_index},
        {"value", m_setting["value"]},
        {"min", m_min},
        {"max", m_max},
        {"step", m_step},
        {"units", m_units},
        {"type", m_type},
        {"labels", m_labels}};
  };

  // This is how the api will put a setting down (top down)
  bool setFromJson(json setting)
  {
    // Different behavior based on m_type
    float fValue;
    float iValue;
    switch (m_type)
    {
    case floatType:
      fValue = setting["value"];
      if (fValue < m_min)
        fValue = m_min;
      if (fValue > m_max)
        fValue = m_max;
      m_setting["value"] = fValue;
      break;
    case intType:
      iValue = setting["value"];
      if (iValue < m_min)
        iValue = m_min;
      if (iValue > m_max)
        iValue = m_max;
      m_setting["value"] = iValue;
      break;
    case booleanType:
      m_setting["value"] = setting["value"];
      break;
    }
    return true;
  };

  // This is how an effect will get a boolean setting
  bool getBoolValue()
  {
    return m_setting["value"];
  };

  // This is how an effect will get an integer setting
  int getIntValue()
  {
    return m_setting["value"];
  };

  // This is how an effect will get a float setting  Unit conversions happen here on the way out
  float getFloatValue()
  {
    float rval = m_setting["value"];
    if (m_units == dB)
      rval = SignalBlock::dbToFloat(rval);
    if (m_units == msec)
      rval /= 1000;
    return rval;
  };

  // This is how an effect will set an initial boolean value
  void setBoolValue(bool value)
  {
    m_setting["value"] = value;
  };

  // This is how an effect will set an initial int value
  void setIntValue(int value)
  {
    if (value < m_min)
      value = m_min;
    if (value > m_max)
      value = m_max;

    m_setting["value"] = value;
  };

  // This is how an effect will set and intial float value
  void setFloatValue(float value)
  {
    if (value < m_min)
      value = m_min;
    if (value > m_max)
      value = m_max;

    m_setting["value"] = value;
  };

private:
  json m_setting;
  int m_index;
  float m_min;
  float m_max;
  float m_step;
  Types m_type;
  Units m_units;
  std::vector<std::string> m_labels;
  std::string m_name;
};