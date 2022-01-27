#pragma once

#include <iostream>
#include "json.hpp"
#include <alsa/asoundlib.h>

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

  const char *typeMap[9] =
      {
          "noteOff",
          "noteOn",
          "polyPressure",
          "controlChange",
          "programChange",
          "channelPressure",
          "pitchBend",
          "systemMessage",
          "unknownType",
  };

public:
  // MidiEvent(unsigned char *buffer)
  // {
  //   switch (buffer[0] & 0xf0)
  //   {
  //   case 0x80:
  //     m_type = noteOff;
  //     break;
  //   case 0x90:
  //     m_type = noteOn;
  //     break;
  //   case 0xa0:
  //     m_type = polyPressure;
  //     break;
  //   case 0xb0:
  //     m_type = controlChange;
  //     break;
  //   case 0xc0:
  //     m_type = programChange;
  //     break;
  //   case 0xd0:
  //     m_type = channelPressure;
  //     break;
  //   case 0xe0:
  //     m_type = pitchBend;
  //     break;
  //   case 0xf0:
  //     m_type = systemMessage;
  //     break;
  //   default:
  //     m_type = unknownType;
  //   }
  //   if (m_type != unknownType)
  //   {
  //     m_channel = buffer[0] & 0x0f;
  //     m_note = buffer[1];
  //     m_velocity = buffer[2];
  //   }
  // }

  // This constructor is used when making from an alsa midi event
  MidiEvent(const snd_seq_event_t *ev)
  {
    switch (ev->type)
    {
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF:
      m_type = noteOn;
      m_channel = ev->data.note.channel;
      m_note = ev->data.note.note;
      m_velocity = ev->data.note.velocity;
      break;
    case SND_SEQ_EVENT_CONTROLLER:
      m_type = controlChange;
      m_channel = ev->data.control.channel;
      m_note = ev->data.control.param;
      m_velocity = ev->data.control.value;
      break;
    case SND_SEQ_EVENT_PGMCHANGE:
      m_type = programChange;
      m_channel = ev->data.control.channel;
      m_note = ev->data.control.value;
      m_velocity = ev->data.control.param;
      break;
    default:
      m_type = unknownType;
      m_channel = 0;
      m_note = 0;
      m_velocity = 0;
      break;
    }
  }

  void dump()
  {
    cout << "msg: " << typeMap[m_type] << " chan: " << m_channel << " note: " << m_note << " vel: " << m_velocity << endl;
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