#pragma once

#include "JamNetStuff.hpp"
#include "MessageQueue.hpp"

enum RTJamParameters
{
  paramChanGain1 = 0,
  paramChanGain2,
  paramChanGain3,
  paramChanGain4,
  paramChanGain5,
  paramChanGain6,
  paramChanGain7,
  paramChanGain8,
  paramChanGain9,
  paramChanGain10,
  paramChanGain11,
  paramChanGain12,
  paramChanGain13,
  paramChanGain14,
  paramMasterVol,
  paramInputMonitor,
  paramRoom0,
  paramRoom1,
  paramRoom2,
  paramReverbChanOne,
  paramReverbMix,
  paramRoomChange,
  paramDisconnect,
  paramHPFOn,
  paramHPFOff,
  paramReverbOne,
  paramReverbTwo,
  paramGetConfigJson,
  paramSetEffectConfig,
  paramCount,
  paramSetAudioInput = 1000,
  paramSetAudioOutput,
  paramListAudioConfig,
  paramCheckForUpdate,
  paramRandomCommand,
  paramRebootDevice = 9998,
  paramShutdownDevice = 9999,
};

struct RTJamParam
{
  RTJamParameters param;
  int iValue;
  int iValue2;
  float fValue;
  char sValue[1024];
};

class ParamData
{
public:
  ParamData() : m_queue("/rtjamParams", sizeof(RTJamParam))
  {
  }

  ~ParamData()
  {
  }

  void flush()
  {
    m_queue.flush();
  }
  void send(RTJamParam *param)
  {
    m_queue.send(param, sizeof(RTJamParam));
  }
  void receive(RTJamParam *param)
  {
    m_queue.recv(param, sizeof(RTJamParam));
  }

private:
  CMessageQueue m_queue;
};
