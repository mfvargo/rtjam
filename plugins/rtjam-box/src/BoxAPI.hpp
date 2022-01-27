#pragma once

#include "LevelData.hpp"
#include "ParamData.hpp"
#include "json.hpp"
#include <fastcgi++/request.hpp>
#include <fastcgi++/manager.hpp>
#include <iostream>
#include <fstream>
#include "EffectFactory.hpp"
#include "MidiEvent.hpp"

#ifndef GIT_HASH
#define GIT_HASH "No Githash in Makefile"
#endif

using namespace std;
using json = nlohmann::json;

class BoxAPI : public Fastcgipp::Request<char>
{
public:
  BoxAPI() : Fastcgipp::Request<char>(50 * 1024)
  {
  }
  static string s_token;            // jam unit's token
  static ParamData s_paramData;     // parameter data msq queue to rtjam-sound
  static LevelData s_levelData;     // shared memory with rtjam-sound and rtjam-midi
  static uint64_t s_jsonTimeStamp;  // timestamp of last pedal update
  static bool s_cmdOutputDirty;     // is the cmdOuput dirty
  static string s_cmdOuput;         // output of last cmd
  static bool s_pedalTypesDirty;    // has somebody requested pedalTypes be updated
  static bool s_audioHardwareDirty; // Audio hardware config has been requested

private:
  RTJamLevels m_jamLevels;

  void doGet()
  {
    // This get called on GETs
    if (environment().requestUri.find("params") != string::npos)
    {
      getParamForm();
    }
    else if (environment().requestUri.find("config") != string::npos)
    {
      getConfig();
    }
    else if (environment().requestUri.find("collect") != string::npos)
    {
      getCollect();
    }
    else
    {
      getLevels();
    }
  }
  void getConfig()
  {
    out << s_levelData.m_pJsonInfo;
  }
  void getLevels()
  {
    memcpy(&m_jamLevels, s_levelData.m_pJamLevels, sizeof(RTJamLevels));
    json result = {
        {"jamUnitToken", s_token},
        {"gitHash", GIT_HASH},
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
        {"midiDevice", s_levelData.m_pRingBuffer->deviceName},
        {"players", json::array()},
    };
    for (int i = 0; i < MAX_JAMMERS; i++)
    {
      result["players"].push_back({
          {"clientId", m_jamLevels.clientIds[i]},
          {"depth", m_jamLevels.bufferDepths[i * 2] * 40},
          {"level0", m_jamLevels.channelLevels[i * 2]},
          {"level1", m_jamLevels.channelLevels[(i * 2) + 1]},
          {"peak0", m_jamLevels.peakLevels[i * 2]},
          {"peak1", m_jamLevels.peakLevels[(i * 2) + 1]},
      });
    }
    out << result.dump(2);
  }

  void getCollect()
  {
    json result = {};
    // Get shared memory data local
    memcpy(&m_jamLevels, s_levelData.m_pJamLevels, sizeof(RTJamLevels));

    // Build the level data
    result["levels"] = {
        {"jamUnitToken", s_token},
        {"gitHash", GIT_HASH},
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
        {"midiDevice", s_levelData.m_pRingBuffer->deviceName},
        {"players", json::array()},
    };
    for (int i = 0; i < MAX_JAMMERS; i++)
    {
      result["levels"]["players"].push_back({
          {"clientId", m_jamLevels.clientIds[i]},
          {"depth", m_jamLevels.bufferDepths[i * 2] * 40},
          {"level0", m_jamLevels.channelLevels[i * 2]},
          {"level1", m_jamLevels.channelLevels[(i * 2) + 1]},
          {"peak0", m_jamLevels.peakLevels[i * 2]},
          {"peak1", m_jamLevels.peakLevels[(i * 2) + 1]},
      });
    }

    // Now build the pedalTypes
    if (s_pedalTypesDirty)
    {
      s_pedalTypesDirty = false;
      result["pedalTypes"] = s_PedalTypes;
    }

    if (s_cmdOutputDirty)
    {
      s_cmdOutputDirty = false;
      result["cmdOutput"] = s_cmdOuput;
    }

    if (s_audioHardwareDirty)
    {
      s_audioHardwareDirty = false;
      result["audioHardware"] = {
          {"driver", execMyCommand("cat soundin.cfg")},
          {"cards", execMyCommand("aplay -l")}};
    }

    // Now update board config if it has changed
    if (s_jsonTimeStamp != m_jamLevels.jsonTimeStamp)
    {
      try
      {
        s_jsonTimeStamp = m_jamLevels.jsonTimeStamp;
        result["pedalInfo"] = json::parse(s_levelData.m_pJsonInfo);
        // We need to forward the pedalboard json data
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

    // Lastly collect any midi messages that may have come in
    result["midiMessages"] = json::array();
    while (s_levelData.m_pRingBuffer->readIdx != s_levelData.m_pRingBuffer->writeIdx)
    {
      // There is something in the ring buffer
      unsigned char *pBuf = &s_levelData.m_pRingBuffer->ringBuffer[s_levelData.m_pRingBuffer->readIdx++ * 28];
      // Construct a MidiEvent using the pBuf
      MidiEvent event((const snd_seq_event_t *)pBuf);
      result["midiMessages"].push_back(event.toJson().dump());
      s_levelData.m_pRingBuffer->readIdx % 32;
    }

    out << result.dump(2);
  }

  void setParam()
  {
    RTJamParam param;
    memset(&param, 0x00, sizeof(RTJamParam));
    for (const auto &post : environment().posts)
    {
      if (post.first == "command")
      {
        param.param = (RTJamParameters)atoi(post.second.c_str());
      }
      if (post.first == "sValue")
      {
        snprintf(param.sValue, sizeof(param.sValue) - 1, "%s", post.second.c_str());
      }
      if (post.first == "fValue")
      {
        param.fValue = atof(post.second.c_str());
      }
      if (post.first == "iValue1")
      {
        param.iValue = atoi(post.second.c_str());
      }
      if (post.first == "iValue2")
      {
        param.iValue2 = atoi(post.second.c_str());
      }
    }
    if (param.param > paramCount)
    {
      // This is a command to be handled by rtjam-box, not passed to sound engine
      cout << "svalue: " << param.sValue << endl;
      switch (param.param)
      {
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
        out << execMyCommand("aplay -l");
        s_audioHardwareDirty = true;
        break;
      case paramCheckForUpdate:
        out << execMyCommand("./checkupdate.bash");
        break;
      case paramRebootDevice:
        out << execMyCommand("reboot 0");
        break;
      case paramShutdownDevice:
        out << execMyCommand("shutdown now");
        break;
      case paramRandomCommand:
        out << execMyCommand(param.sValue);
        s_cmdOutputDirty = true;
        break;
      case paramGetPedalTypes:
      {
        json pedalTypes(s_PedalTypes);
        s_pedalTypesDirty = true;
        out << pedalTypes.dump(2);
      }
      break;
      default:
        out << "Unknown Command";
      }
    }
    else
    {
      // This is a sound engine parameter
      s_paramData.flush();
      s_paramData.send(&param);
      out << "OK";
      // getParamForm();
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
    s_cmdOuput = result;
    return result;
  }

  void getParamForm()
  {
    out << "<h1>Set Param Form</h1>"
           "<form method='post' "
           "enctype='application/x-www-form-urlencoded' "
           "accept-charset='utf-8'>"
           "Command: <input type='text' name='command' value='21'/><br />"
           "sValue: <input type='text' name='sValue' value='music.basscleftech.com'/><br />"
           "fValue: <input type='text' name='fValue' value='0.0'/><br />"
           "iValue1: <input type='text' name='iValue1' value='7891'/><br />"
           "iValue2: <input type='text' name='iValue2' value='5025'/><br />"
           "<input type='submit' name='submit' value='submit' />"
           "</form>";

    // "<form action='/rtjambox/param' method='put' "
    //     "enctype='application/x-www-form-urlencoded' "
    //     "accept-charset='utf-8'>"
  }
  void writeBody()
  {
    using Fastcgipp::Encoding;
    switch (environment().requestMethod)
    {
    case Fastcgipp::Http::RequestMethod::GET:
      doGet();
      break;
    case Fastcgipp::Http::RequestMethod::POST:
      setParam();
      break;
    default:
      break;
    }
  }

  bool response()
  {
    try
    {
      using Fastcgipp::Encoding;

      out << "Set-Cookie: echoCookie=" << Encoding::URL << "<\"rtjam_cook\">;"
          << Encoding::NONE << "; path=/\n";
      out << "Content-Type: text/html; charset=utf-8\r\n\r\n";
      out << "<!DOCTYPE html>\n"
             "<html>"
             "<head>"
             "<meta charset='utf-8' />"
             "<title>fastcgi++: Echo</title>"
             "</head>";
      out << "<body>";

      writeBody();

      out << "</body></html>";
      return true;
    }
    catch (exception &e)
    {
      clog << e.what() << endl;
    }
    return true;
  }
};