#ifndef PARAMDATA_RTJAM_HPP
#define PARAMDATA_RTJAM_HPP

#include "JamNetStuff.hpp"
#include "MessageQueue.hpp"


struct RTJamParam {
  int param;
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
