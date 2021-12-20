#pragma once

#include "ChatRobotBase.hpp"
#include "easywsclient.hpp"
#include "JamNetStuff.hpp"
#include "ParamData.hpp"
#include "LevelData.hpp"
#include "EffectFactory.hpp"
#include "MidiEvent.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

uint64_t g_pollingInterval = 30 * 1000 * 1000; // Default is 30 seconds
uint64_t g_setIntervalTime = 0;
uint64_t g_lastPollUpdate = 0;

class UnitChatRobot : public ChatRobotBase
{
public:
  bool isRunning = true;

  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token, LevelData *pLevelData, ParamData *pParamData)
  {
    cout << "init on UnitChatRobot" << endl;
    m_token = token;
    m_pLevelData = pLevelData;
    m_pParamData = pParamData;
    ChatRobotBase::init(url, m_token);
    g_lastPollUpdate = JamNetStuff::getMicroTime();
    m_jsonTimeStamp = JamNetStuff::getMicroTime();
    g_setIntervalTime = JamNetStuff::getMicroTime();
  };

  // This loop will drive the chat bot.
  void readMessages()
  {
    while (isRunning && ws != NULL && ws->getReadyState() != WebSocket::CLOSED)
    {
      ws->poll(50);
      // This next line uses the crazy C++ functor thing where you can pass in an
      // object and it will call it's operator ().
      ws->dispatch(*this);
      doInterPollStuff();
      sendMidiMessages();
    }
  }

  void doMessage(const json &msgRaw) override
  {
    try
    {
      string msgString = msgRaw["message"];
      json msg = json::parse(msgString);
      RTJamParam param;
      memset(&param, 0x00, sizeof(RTJamParam));
      param.param = msg["param"];
      if (!msg["sValue"].empty())
      {
        snprintf(param.sValue, sizeof(param.sValue) - 1, "%s", msg["sValue"].get<string>().c_str());
      }
      if (!msg["iValue1"].empty())
      {
        param.iValue = msg["iValue1"];
      }
      if (!msg["iValue2"].empty())
      {
        param.iValue2 = msg["iValue2"];
      }
      if (!msg["fValue"].empty())
      {
        param.fValue = msg["fValue"];
      }
      if (param.param > paramCount)
      {
        // This is an api call rtjam-box will handle
        doLocalCommand(param);
      }
      else
      {
        // This api call is to be passed to rtjam-sound
        m_pParamData->send(&param);
      }
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

  void doLocalCommand(RTJamParam &param)
  {
    // This is a command that is to be handled by rtjam-box
    json msg = {{"speaker", "UnitChatRobot"}};
    switch (param.param)
    {
    case paramGetPedalTypes:
    {
      msg["pedalTypes"] = s_PedalTypes;
      sendMessage("say", msg.dump());
      break;
    }
    case paramSetUpdateInterval:
    {
      if (param.iValue > 50 && param.iValue < 60000)
      {
        g_pollingInterval = param.iValue * 1000; // Convert to microseconds
        g_setIntervalTime = JamNetStuff::getMicroTime();
      }
      break;
    }
    case paramSetAudioInput:
    {
      std::ofstream outfile("soundin.cfg");
      outfile << param.sValue;
    }
    break;
    case paramSetAudioOutput:
    {
      std::ofstream outfile("soundout.cfg");
      outfile << param.sValue;
    }
    break;
    case paramListAudioConfig:
    {
      msg["audioHardware"] = {
          {"driver", execMyCommand("cat soundin.cfg")},
          {"cards", execMyCommand("aplay -l")}};
      sendMessage("say", msg.dump());
    }
    break;
    case paramCheckForUpdate:
      msg["cmdOutput"] = execMyCommand("./checkupdate.bash");
      sendMessage("say", msg.dump());
      break;
    case paramRebootDevice:
      msg["cmdOutput"] = execMyCommand("reboot 0");
      sendMessage("say", msg.dump());
      break;
    case paramShutdownDevice:
      msg["cmdOutput"] = execMyCommand("shutdown now");
      sendMessage("say", msg.dump());
      break;
    case paramRandomCommand:
      string result = execMyCommand(param.sValue);
      msg["cmdOutput"] = result;
      sendMessage("say", msg.dump());
      break;
    }
  }

  string execMyCommand(string cmd)
  {
    array<char, 128> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
      return "popen() failed!";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
      result += buffer.data();
    }
    return result;
  }

  void doInterPollStuff()
  {
    if (JamNetStuff::getMicroTime() - g_lastPollUpdate > g_pollingInterval)
    {
      g_lastPollUpdate = JamNetStuff::getMicroTime();
      // reset poll interval every 3 minutes
      if ((g_lastPollUpdate - g_setIntervalTime) > 3 * 60 * 1000 * 1000)
      {
        g_pollingInterval = 30 * 1000 * 1000; // switch to 30 second interval
      }
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
          {"leftRoomMute", m_jamLevels.leftRoomMute},
          {"rightRoomMute", m_jamLevels.rightRoomMute},
          {"beat", m_jamLevels.beat},
          {"connected", m_jamLevels.isConnected},
          {"jsonTimeStamp", m_jamLevels.jsonTimeStamp},
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
      if (m_jsonTimeStamp != m_jamLevels.jsonTimeStamp)
      {
        try
        {
          m_jsonTimeStamp = m_jamLevels.jsonTimeStamp;
          // We need to forward the pedalboard json data
          json pedals = {{"speaker", "UnitChatRobot"}};
          pedals["pedalInfo"] = json::parse(m_pLevelData->m_pJsonInfo);
          sendMessage("say", pedals.dump());
        }
        catch (json::exception &e)
        {
          cerr << "error parsing pedalboard" << endl;
          cerr << e.what() << endl;
        }
        catch (...)
        {
          cerr << "error parsing pedalboard" << endl;
        }
      }
    }
  }

  void sendMidiMessages()
  {
    // See if there are any midi messages in the ring buffer
    while (m_pLevelData->m_pRingBuffer->readIdx != m_pLevelData->m_pRingBuffer->writeIdx)
    {
      // There is something in the ring buffer
      unsigned char *pBuf = &m_pLevelData->m_pRingBuffer->ringBuffer[m_pLevelData->m_pRingBuffer->readIdx++ * 3];
      // Construct a MidiEvent using the pBuf
      MidiEvent event(pBuf);
      sendMessage("say", event.toJson().dump());
      // Wrap the ring buffer index
      m_pLevelData->m_pRingBuffer->readIdx % 32;
    }
  }

private:
  // member variables
  string m_token;
  RTJamLevels m_jamLevels;
  LevelData *m_pLevelData;
  ParamData *m_pParamData;
  uint64_t m_jsonTimeStamp;
};