#ifndef PLUGIN_RTJAM_HPP
#define PLUGIN_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "JamDirectory.hpp"
#include "Settings.hpp"
#include "LevelData.hpp"

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

class PluginRTJam {
  public:
     PluginRTJam();
    ~PluginRTJam();
    void run(const float** inputs, float** outputs, uint32_t frames);
    void connect(const char* host, int port, uint32_t id);

  private:
    JamNetStuff::JamSocket m_jamSocket;
    JamNetStuff::JamMixer m_jamMixer;
    JamNetStuff::StreamTimeStats leftInput;
    JamNetStuff::StreamTimeStats rightInput;
    float* m_outputs[NUM_OUTPUTS];
    LevelData m_levelData;
    uint64_t m_framecount;
};

#endif
