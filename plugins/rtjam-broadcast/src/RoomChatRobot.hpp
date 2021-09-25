#pragma once

#include "ChatRobotBase.hpp"
#include "easywsclient.hpp"
#include "JamNetStuff.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

class RoomChatRobot : public ChatRobotBase
{
public:
  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token, JamNetStuff::JamSocket *pJamSocket)
  {
    ChatRobotBase::init(url, token);
    m_lastLatencyUpdate = JamNetStuff::getMicroTime();
  };


  void doMessage(const json &msg)
  {
    // This is a message from the room chat.
    string command = msg["message"];

    // See if this message is a command for the robot
    int idx = command.find("!tempo");
    if (idx != string::npos)
    {
      // tempo command to change the room tempo
      // cout << "Tempo command" << endl;
      if (command.size() > strlen("!tempo"))
      {
        int tempo = atoi(command.substr(7).c_str());
        if (tempo > 0)
        {
          m_pJamSocket->setTempo(tempo);
        }
      }
      json resp = {{"speaker", "RoomChatRobot"}};
      resp["tempo"] = m_pJamSocket->getTempo();
      sendMessage("say", resp.dump());
      return;
    }
  }
  void doInterPollStuff()
  {
    if (JamNetStuff::getMicroTime() - m_lastLatencyUpdate > 2000000)
    {
      json resp = {{"speaker", "RoomChatRobot"}};
      resp["latency"] = m_pJamSocket->getLatency();
      sendMessage("say", resp.dump());
      m_lastLatencyUpdate = JamNetStuff::getMicroTime();
    }
  }
private:
  // member variables
  JamNetStuff::JamSocket *m_pJamSocket;
  uint64_t m_lastLatencyUpdate;

};