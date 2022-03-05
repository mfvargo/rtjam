#ifndef PLUGIN_RTJAM_HPP
#define PLUGIN_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "LevelData.hpp"
#include "LightData.hpp"
#include "ParamData.hpp"
#include <thread>
#include <vector>
#include "PedalBoard.hpp"
#include <jack/jack.h>

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

class JamEngine
{
public:
  JamEngine();
  ~JamEngine();
  void init();
  void syncLevels();
  void syncConfigData();
  void run(const float **inputs, float **outputs, uint32_t frames);
  void sendMidiEvent(unsigned char *buffer);
  void getParams();
  void connect(const char *host, int port, uint32_t id);
  void disconnect();
  void setJackClient(jack_client_t *client) { m_jackClient = client; }

private:
  PedalBoard m_pedalBoards[2];
  LightColors dbToColor(float value);
  JamNetStuff::JamSocket m_jamSocket;
  JamNetStuff::JamMixer m_jamMixer;
  JamNetStuff::StreamTimeStats m_leftInput;
  JamNetStuff::StreamTimeStats m_rightInput;
  JamNetStuff::StreamTimeStats m_leftRoomInput;
  JamNetStuff::StreamTimeStats m_rightRoomInput;
  float *m_outputs[NUM_OUTPUTS];
  RTJamParam m_param;
  LightData m_lightData;
  uint64_t m_framecount;
  JamNetStuff::MicroTimer m_conTimer;
  RTJamLevels m_levels;
  std::vector<std::thread> m_threads;
  LevelData m_levelData;
  ParamData m_paramData;
  BiQuadFilter m_inputDCremoval[2];
  bool m_LeftRoomMute;
  bool m_RightRoomMute;
  jack_client_t *m_jackClient;
};

#endif
