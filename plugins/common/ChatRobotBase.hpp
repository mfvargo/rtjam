#pragma once

#include <iostream>
#include <string>
#include <assert.h>
#include "json.hpp"
#include "easywsclient.hpp"

using namespace std;
using easywsclient::WebSocket;
using json = nlohmann::json;

class ChatRobotBase
{
public:
  // derived classes need to implement these
  virtual void doMessage(const json &msg) = 0;

  enum RoomState
  {
    room_init,
    room_creating,
    room_joining,
    room_processing
  };

  bool isConnected() {
    return (ws != NULL && ws->getReadyState() != WebSocket::CLOSED);
  }

  // This is called to intialize the ChatRobot and join the room
  void init(string url, string token)
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
    m_msgId = 1;
    m_state = room_init;
    createRoom();
  };


  // The operator () lets me pass a ChatRobot to the websocket dispatch function
  // So this is the callback when a message comes in on the websocket
  void operator()(const std::string &message)
  {
    try
    {
      // cout << message << endl;
      // Handle primus protocol messages
      if (message.find("primus:") != string::npos)
      {
        return doPrimus(message);
      }
      // Action hero chat are json encoded
      json msg = json::parse(message);
      // cout << msg.dump(2) << endl;
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

  void sendMessage(string event, string message)
  {
    if (isConnected()) {
      json msg = {{"event", event},
                  {"message", message},
                  {"room", m_token},
                  {"messageId", m_msgId++}};
      // cout << msg.dump(2) << endl;
      ws->send(msg.dump());
    }
  }



protected:
  // member variables
  WebSocket::pointer ws = NULL;
  string m_token;
  RoomState m_state;
  int m_msgId;
  int m_responseId;


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

  // This is primus protocol stuff
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
  // This runs the state machine to connect to the room.
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
};