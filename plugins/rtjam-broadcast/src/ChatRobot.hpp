#pragma once

#include <iostream>
#include <string>
#include <assert.h>
#include "json.hpp"

#include "easywsclient.hpp"
#include "JamNetStuff.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

class ChatRobot
{
public:
  void init(string url, string token, JamNetStuff::JamSocket *pJamSocket)
  {
    ws = WebSocket::from_url(url);
    assert(ws);
    m_token = token;
    m_pJamSocket = pJamSocket;
    m_msgId = 1;
    createRoom();
    joinRoom();
  };

  void operator()(const std::string &message)
  {
    try
    {
      if (message.find("primus:") != string::npos)
      {
        return doPrimus(message);
      }
      json msg = json::parse(message);
      cout << msg.dump(2) << endl;
      string command = msg["message"];
      int idx = command.find("!tempo");
      if (idx != string::npos)
      {
        // tempo command
        cout << "Tempo command" << endl;
        if (command.size() > strlen("!tempo"))
        {
          int tempo = atoi(command.substr(7).c_str());
          if (tempo > 0)
          {
            m_pJamSocket->setTempo(tempo);
          }
        }
        string resp = "tempo: ";
        resp += std::to_string(m_pJamSocket->getTempo());
        sendMessage("say", resp);
        return;
      }
      if (command == "!latency")
      {
        string resp = m_pJamSocket->getLatency();
        sendMessage("say", resp);
      }
    }
    catch (...)
    {
      cout << "error parsing: " << message << endl;
    }
  }

  void createRoom()
  {
    json msg = {
        {"event", "action"},
        {"params", {
                       {"name", m_token},
                       {"action", "createChatRoom"},
                   }},
        {"messageId", m_msgId++}};
    cout << "createRoom:" << endl
         << msg.dump(2) << endl;
    ws->send(msg.dump());
  }

  void joinRoom()
  {
    json msg = {{"event", "roomAdd"}, {"room", m_token}, {"messageId", m_msgId++}};
    cout << "joinRoom:" << endl
         << msg.dump(2) << endl;
    ws->send(msg.dump());
  }

  void sendMessage(string event, string message)
  {
    json msg = {{"event", event},
                {"message", message},
                {"room", m_token},
                {"messageId", m_msgId++}};
    cout << msg.dump(2) << endl;
    ws->send(msg.dump());
  }

  void readMessages()
  {
    while (ws->getReadyState() != WebSocket::CLOSED)
    {
      ws->poll(1000);
      // This next line uses the crazy C++ functor thing where you can pass in an
      // object and it will call it's operator ().
      ws->dispatch(*this);
    }
  }

private:
  void doPrimus(const string &message)
  {
    string msg = message;
    int idx = message.find("::ping");
    if (idx != string::npos)
    {
      msg.replace(idx, 6, "::pong");
      ws->send(msg);
      return;
    }
    // Additional primus message checks go below as other message.find() calls
    // idx = message.find("::otherThing")...
  }

  WebSocket::pointer ws;
  string m_token;
  int m_msgId;
  JamNetStuff::JamSocket *m_pJamSocket;
};