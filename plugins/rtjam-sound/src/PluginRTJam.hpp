#ifndef PLUGIN_RTJAM_HPP
#define PLUGIN_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "LevelData.hpp"
#include "LightData.hpp"
#include "ParamData.hpp"
#include <thread>
#include <vector>
#include "MonoVerb.hpp"
#include "HighPassFilter.hpp"
#include "Delay.hpp"
#include "Distortion.hpp"
#include "EffectChain.hpp"
#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

class PluginRTJam
{
public:
  PluginRTJam();
  ~PluginRTJam();
  void init();
  void syncLevels();
  void syncConfigData();
  void run(const float **inputs, float **outputs, uint32_t frames);
  void paramFlush();
  void getParams();
  void connect(const char *host, int port, uint32_t id);
  void disconnect();

private:
  EffectChain m_effectChains[2];
  MonoVerb m_reverbs[2];
  HighPassFilter m_filters[2];
  SigmaDelay m_delays[2];
  Distortion m_distortions[2];
  LightColors dbToColor(float value);
  JamNetStuff::JamSocket m_jamSocket;
  JamNetStuff::JamMixer m_jamMixer;
  JamNetStuff::StreamTimeStats leftInput;
  JamNetStuff::StreamTimeStats rightInput;
  float *m_outputs[NUM_OUTPUTS];
  ParamData m_paramData;
  RTJamParam m_param;
  LightData m_lightData;
  uint64_t m_framecount;
  RTJamLevels m_levels;
  std::vector<std::thread> m_threads;
};

#endif
