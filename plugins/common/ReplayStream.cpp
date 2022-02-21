#include "JamNetStuff.hpp"

namespace JamNetStuff
{
  string ReplayStream::readOpen(const char *filename)
  {
    m_timeOffset = 0;
    // Open the file
    if (m_infile.is_open())
    {
      // If file is open, rewind to the beginning
      m_infile.seekg(0);
      return "playing";
    }
    m_infile.open(filename, ios::in | ios::binary);
    if (!m_infile.good())
    {
      return "error opening file";
    }
    // now that we are here, let's load the first packet
    if (!readPacket())
    {
      // Could not read first packet.  close
      close();
      return "error reading file";
    }
    uint64_t now = getMicroTime();
    m_timeOffset = now - m_timeStamp;
    return "playing";
  }

  string ReplayStream::writeOpen(const char *filename)
  {
    // Open the file
    if (m_outfile.is_open())
    {
      return "recording";
    }
    m_outfile.open(filename, ios::out | ios::binary);
    if (!m_outfile.good())
    {
      return "error opening file";
    }
    return "recording";
  }

  string ReplayStream::close()
  {
    if (m_infile.is_open())
    {
      m_infile.close();
    }
    if (m_outfile.is_open())
    {
      m_outfile.close();
    }
    return "idle";
  }

  bool ReplayStream::packetReady()
  {
    if (!m_infile.is_open())
    {
      return false;
    }
    if (m_infile.peek() == EOF)
    {
      m_infile.close();
      return false;
    }
    return (getMicroTime() > m_timeStamp + m_timeOffset);
  }

  bool ReplayStream::readPacket()
  {
    if (!m_infile.is_open())
    {
      return false;
    }
    uint64_t timeStamp;
    m_infile.read((char *)&timeStamp, sizeof(timeStamp));
    m_timeStamp = be64toh(timeStamp);
    uint16_t cnt;
    m_infile.read((char *)&cnt, sizeof(cnt));
    cnt = ntohs(cnt);
    if (m_infile.good() && cnt > 0 && cnt < sizeof(JamMessage))
    {
      m_infile.read((char *)m_packet.getPacket(), cnt);
      if (m_infile.good())
      {
        m_packet.decodeHeader(cnt);
        // m_packet.dumpPacket("replay: ");
        return true;
      }
    }
    return false;
  }

  bool ReplayStream::writePacket(JamNetStuff::JamPacket *packet)
  {
    // Write the data to the file
    if (m_outfile.is_open())
    {
      // we write the packet to the file
      uint64_t tStamp = htobe64(JamNetStuff::getMicroTime()); // Timestamp
      m_outfile.write((const char *)&tStamp, sizeof(tStamp));
      uint16_t cnt = htons(packet->getSize()); // Sizeof the packet
      m_outfile.write((const char *)&cnt, sizeof(cnt));
      m_outfile.write((const char *)packet->getPacket(), packet->getSize()); // The packet itself (encoded)
      return m_outfile.good();
    }
    return false;
  }

  JamNetStuff::JamPacket *ReplayStream::getJamPacket()
  {
    return &m_packet;
  }

  JamNetStuff::JamPacket *ReplayStream::getPlayBackMix()
  {
    return NULL;
  }
}