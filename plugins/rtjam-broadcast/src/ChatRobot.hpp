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
  enum RoomState
  {
    room_init,
    room_creating,
    room_joining,
    room_processing
  };

  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token, JamNetStuff::JamSocket *pJamSocket)
  {
    if (ws != NULL)
    {
      delete ws;
    }
    ws = WebSocket::from_url(url);
    if (ws == NULL)
    {
      // failed to connect.
      return;
    }
    m_token = token;
    m_pJamSocket = pJamSocket;
    m_msgId = 1;
    m_lastLatencyUpdate = JamNetStuff::getMicroTime();
    m_state = room_init;
    // TODO:  need to wait for responses on these commands
    createRoom();
  };

  // This loop will drive the chat bot.
  void readMessages()
  {
    while (ws != NULL && ws->getReadyState() != WebSocket::CLOSED)
    {
      ws->poll(1000);
      // This next line uses the crazy C++ functor thing where you can pass in an
      // object and it will call it's operator ().
      ws->dispatch(*this);
      // see if we need to update latency to room
      if (JamNetStuff::getMicroTime() - m_lastLatencyUpdate > 5000000)
      {
        string resp = m_pJamSocket->getLatency();
        sendMessage("say", resp);
        m_lastLatencyUpdate = JamNetStuff::getMicroTime();
      }
    }
  }

  // The operator () lets me pass a ChatRobot to the websocket dispatch function
  // So this is the callback when a message comes in on the websocket
  void operator()(const std::string &message)
  {
    try
    {
      // Handle primus protocol messages
      if (message.find("primus:") != string::npos)
      {
        return doPrimus(message);
      }
      // Action hero chat are json encoded
      json msg = json::parse(message);
      cout << msg.dump(2) << endl;
      if (msg["context"].size() == 0)
      {
        return;
      }
      string context = msg["context"];
      if (context == "response")
      {
        doResponse(msg);
      }
      if (context == "user")
      {
        doMessage(msg);
      }
    }
    catch (json::exception &e)
    {
      cerr << e.what() << endl;
    }
    catch (...)
    {
      cout << "error parsing: " << message << endl;
    }
  }

private:
  // member variables
  WebSocket::pointer ws = NULL;
  string m_token;
  RoomState m_state;
  int m_msgId;
  JamNetStuff::JamSocket *m_pJamSocket;
  int m_responseId;
  uint64_t m_lastLatencyUpdate;

private:
  void createRoom()
  {
    m_state = room_creating;
    m_responseId = m_msgId;
    json msg = {
        {"event", "action"},
        {"params", {
                       {"name", m_token},
                       {"action", "createChatRoom"},
                   }},
        {"messageId", m_msgId++}};
    // cout << "createRoom:" << endl
    //      << msg.dump(2) << endl;
    ws->send(msg.dump());
  }

  void joinRoom()
  {
    m_state = room_joining;
    m_responseId = m_msgId;
    json msg = {{"event", "roomAdd"}, {"room", m_token}, {"messageId", m_msgId++}};
    // cout << "joinRoom:" << endl
    //      << msg.dump(2) << endl;
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
  void doResponse(const json &msg)
  {
    if (msg["messageId"] == m_responseId)
    {
      // This is a response we are waiting for
      switch (m_state)
      {
      case room_creating:
        // join the room
        joinRoom();
        break;
      case room_joining:
        m_state = room_processing;
        break;
      }
    }
  }
  void doMessage(const json &msg)
  {
    // This is a message from the room chat.
    string command = msg["message"];

    // See if this message is a command for the robot
    int idx = command.find("!tempo");
    if (idx != string::npos)
    {
      // tempo command to change the room tempo
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
      return;
    }
  }
};