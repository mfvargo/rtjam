#pragma once

#include "ChatRobotBase.hpp"
#include "easywsclient.hpp"
#include "JamNetStuff.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

class UnitChatRobot : public ChatRobotBase
{
public:
  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token)
  {
    ChatRobotBase::init(url, token);
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

  void doMessage(const json &msg) override
  {
    // This is a message from the room chat.
    string command = msg["message"];
    cout << msg.dump(2) << endl;
    return;
  }
  void doInterPollStuff()
  {
    if (JamNetStuff::getMicroTime() - m_lastPollUpdate > 20000000)
    {
      cout << "Interpol: " << m_state << endl;
      json resp = {{"speaker", "UnitChatRobot"}};
      resp["stuff"] = "I message, therefore I am!";
      sendMessage("say", resp.dump());
      m_lastPollUpdate = JamNetStuff::getMicroTime();
    }
  }

private:
  // member variables
  uint64_t m_lastPollUpdate;
};