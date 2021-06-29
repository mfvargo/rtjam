#ifndef PLUGIN_RTJAM_HPP
#define PLUGIN_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "LevelData.hpp"
#include "LightData.hpp"
#include "ParamData.hpp"
#include <thread>
#include <vector>
#include "MonoVerb.hpp"

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

class PluginRTJam
{
public:
  PluginRTJam();
  ~PluginRTJam();
  void init();
  void syncLevels();
  void run(const float **inputs, float **outputs, uint32_t frames);
  void paramFlush();
  void getParams();
  void connect(const char *host, int port, uint32_t id);
  void disconnect();

private:
  MonoVerb m_reverbs[2];
  JamNetStuff::HighPassFilter filters[2];
  float dbToFloat(float value);
  LightColors dbToColor(float value);
  JamNetStuff::JamSocket m_jamSocket;
  JamNetStuff::JamMixer m_jamMixer;
  JamNetStuff::StreamTimeStats leftInput;
  JamNetStuff::StreamTimeStats rightInput;
  float *m_outputs[NUM_OUTPUTS];
  LevelData m_levelData;
  ParamData m_paramData;
  RTJamParam m_param;
  LightData m_lightData;
  uint64_t m_framecount;
  RTJamLevels m_levels;
  std::vector<std::thread> m_threads;
};

#endif
