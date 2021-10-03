#pragma once

#include "ChatRobotBase.hpp"
#include "easywsclient.hpp"
#include "JamNetStuff.hpp"
#include "ParamData.hpp"
#include "LevelData.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

class UnitChatRobot : public ChatRobotBase
{
public:
  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token, LevelData* pLevelData)
  {
    m_token = token;
    m_pLevelData = pLevelData;
    ChatRobotBase::init(url, m_token);
    m_lastPollUpdate = JamNetStuff::getMicroTime();
  };

  // This loop will drive the chat bot.
  void readMessages()
  {
    while (ws != NULL && ws->getReadyState() != WebSocket::CLOSED)
    {
      ws->poll(50);
      // This next line uses the crazy C++ functor thing where you can pass in an
      // object and it will call it's operator ().
      ws->dispatch(*this);
      doInterPollStuff();
    }
  }

  void doMessage(const json &msgRaw) override
  {
    try {
      string msgString = msgRaw["message"];
      json msg = json::parse(msgString);
      cout << msg.dump(2) << endl;
      RTJamParam param;
      memset(&param, 0x00, sizeof(RTJamParam));
      param.param = msg["param"];
      // if (msg["sValue"] != NULL) {
      //   string sValue = msg["sValue"];
      //   snprintf(param.sValue, sizeof(param.sValue) - 1, "%s", sValue.c_str());
      // }
      // if (msg["iValue1"] != NULL) param.iValue = msg["iValue1"];
      // if (msg["iValue2"] != NULL) param.iValue2 = msg["iValue2"];
      if (msg["fValue"] != NULL) param.fValue = msg["fValue"];
      m_paramData.send(&param);
    }
      catch (json::exception &e)
    {
      cerr << e.what() << endl;
    }
      catch (...)
    {
      cerr << "error parsing command" << endl;
    }
    return;
  }
  void doInterPollStuff()
  {
    if (JamNetStuff::getMicroTime() - m_lastPollUpdate > 200000)
    {
      memcpy(&m_jamLevels, m_pLevelData->m_pJamLevels, sizeof(RTJamLevels));
      json levels = {{"speaker", "UnitChatRobot"}};
      levels["levelEvent"] = {
          {"jamUnitToken", m_token},
          {"masterLevel", m_jamLevels.masterLevel},
          {"peakMaster", m_jamLevels.peakMaster},
          {"inputLeft", m_jamLevels.inputLeft},
          {"inputRight", m_jamLevels.inputRight},
          {"peakLeft", m_jamLevels.peakLeft},
          {"peakRight", m_jamLevels.peakRight},
          {"roomInputLeft", m_jamLevels.roomLevelLeft},
          {"roomInputRight", m_jamLevels.roomLevelRight},
          {"roomPeakLeft", m_jamLevels.roomPeakLeft},
          {"roomPeakRight", m_jamLevels.roomPeakRight},
          {"inputLeftFreq", m_jamLevels.inputLeftFreq},
          {"inputRightFreq", m_jamLevels.inputRightFreq},
          {"leftTunerOn", m_jamLevels.leftTunerOn},
          {"rightTunerOn", m_jamLevels.rightTunerOn},
          {"beat", m_jamLevels.beat},
          {"connected", m_jamLevels.isConnected},
          {"players", json::array()},
      };
      for (int i = 0; i < MAX_JAMMERS; i++)
      {
        levels["levelEvent"]["players"].push_back({
            {"clientId", m_jamLevels.clientIds[i]},
            {"depth", m_jamLevels.bufferDepths[i * 2] * 40},
            {"level0", m_jamLevels.channelLevels[i * 2]},
            {"level1", m_jamLevels.channelLevels[(i * 2) + 1]},
            {"peak0", m_jamLevels.peakLevels[i * 2]},
            {"peak1", m_jamLevels.peakLevels[(i * 2) + 1]},
        });
      }
      sendMessage("say", levels.dump());
      m_lastPollUpdate = JamNetStuff::getMicroTime();
    }
  }

private:
  // member variables
  string m_token;
  uint64_t m_lastPollUpdate;
  RTJamLevels m_jamLevels;
  LevelData* m_pLevelData;
  ParamData m_paramData;
};