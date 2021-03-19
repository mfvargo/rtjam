#ifndef PLUGIN_RTJAM_HPP
#define PLUGIN_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "JamDirectory.hpp"
#include "Settings.hpp"
#include "LevelData.hpp"
#include "ParamData.hpp"
#include <thread>


#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

class PluginRTJam {
  public:
     PluginRTJam();
    ~PluginRTJam();
    void init();
    void syncLevels();
    void run(const float** inputs, float** outputs, uint32_t frames);
    void paramFlush();
    void getParams();
    void connect(const char* host, int port, uint32_t id);

  private:
    float dbToFloat(float value);
    JamNetStuff::JamSocket m_jamSocket;
    JamNetStuff::JamMixer m_jamMixer;
    JamNetStuff::StreamTimeStats leftInput;
    JamNetStuff::StreamTimeStats rightInput;
    float* m_outputs[NUM_OUTPUTS];
    LevelData m_levelData;
    ParamData m_paramData;
    RTJamParam m_param;
    uint64_t m_framecount;
    RTJamLevels m_levels;
    std::vector<std::thread> m_threads;
    Settings m_settings;
    JamNetStuff::JamDirectory m_jamDirectory;
};

#endif
