#pragma once

#include <jack/jack.h>
#include <jack/midiport.h>
#include <iostream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class MidiEvent
{
  enum MessageType
  {
    noteOff,
    noteOn,
    polyPressure,
    controlChange,
    programChange,
    channelPressure,
    pitchBend,
    systemMessage,
    unknownType,
  };

public:
  MidiEvent(unsigned char *buffer)
  {
    switch (buffer[0] & 0xf0)
    {
    case 0x80:
      m_type = noteOff;
      break;
    case 0x90:
      m_type = noteOn;
      break;
    case 0xa0:
      m_type = polyPressure;
      break;
    case 0xb0:
      m_type = controlChange;
      break;
    case 0xc0:
      m_type = programChange;
      break;
    case 0xd0:
      m_type = channelPressure;
      break;
    case 0xe0:
      m_type = pitchBend;
      break;
    case 0xf0:
      m_type = systemMessage;
      break;
    default:
      m_type = unknownType;
    }
    if (m_type != unknownType)
    {
      m_channel = buffer[0] & 0x0f;
      m_note = buffer[1];
      m_velocity = buffer[2];
    }
  }

  void dump()
  {
    cout << "msg: " << m_type << " chan: " << m_channel << " note: " << m_note << " vel: " << m_velocity << endl;
  }

  json toJson()
  {
    json resp = {{"speaker", "UnitChatRobot"}};
    resp["midiEvent"] = {
        {"type", m_type},
        {"channel", m_channel},
        {"note", m_note},
        {"velocity", m_velocity}};
    return resp;
  }

  MessageType m_type;
  unsigned m_note;
  unsigned m_velocity;
  unsigned m_channel;
};