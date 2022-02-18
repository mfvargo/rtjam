#pragma once

#include "JamNetStuff.hpp"

class ReplayStream
{
public:
  bool open(const char *filename)
  {
    timeOffset = 0;
    // Open the file
    if (m_file.is_open())
    {
      // If file is open, rewind to the beginning
      m_file.seekg(0);
      return m_file.good();
    }
    m_file.open(filename, ios::in | ios::binary);
    return m_file.good();
  }

  bool close()
  {
    if (!m_file.is_open())
    {
      return false;
    }
    m_file.close();
    return m_file.good();
  }

  bool readPacket()
  {
    if (!m_file.is_open())
    {
      return false;
    }
    int cnt;
    m_file.read((char *)&cnt, sizeof(cnt));
    if (m_file.good() && cnt > 0 && cnt < sizeof(JamMessage))
    {
      m_file.read((char *)m_packet.getPacket(), cnt);
      if (m_file.good())
      {
        m_packet.decodeHeader(cnt);
        m_packet.dumpPacket("replay: ");
      }
    }
    return false;
  }

  JamNetStuff::JamPacket *getJamPacket()
  {
    return &m_packet;
  }

private:
  JamNetStuff::JamPacket m_packet;
  ifstream m_file;
  int timeOffset;
};