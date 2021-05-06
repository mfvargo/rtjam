#ifndef PARAMDATA_RTJAM_HPP
#define PARAMDATA_RTJAM_HPP

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
      paramReverbMix1,
      paramCount
};

struct RTJamParam {
  RTJamParameters param;
  int iValue;
  int iValue2;
  float fValue;
  char sValue[128];
};

class ParamData {
  public:
     ParamData();
    ~ParamData();

    void flush();
    void send(RTJamParam* param);
    void receive(RTJamParam* param);

  private:
    CMessageQueue m_queue;
};

#endif
