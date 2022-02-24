#include "JamNetStuff.hpp"

namespace JamNetStuff
{
  ReplayStream::ReplayStream()
  {
    m_sendPacket.setClientId(40001); // TODO: fix this later
    m_sendPacket.setIsClient(true);

    for (int i = 0; i < MIX_CHANNELS + 2; i++)
    {
      m_outputs[i] = new float[1024];
    }
  }
  ReplayStream::~ReplayStream()
  {
    for (int i = 0; i < MIX_CHANNELS + 2; i++)
    {
      delete m_outputs[i];
    }
  }
  string ReplayStream::readOpen(const char *filename)
  {
    // for debug
    m_framecount = 0;
    m_timeOffset = 0;
    m_delta = 0;
    m_timer.reset();
    m_mixer.masterVol = 0.707;
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
    if (!m_outfile.open(filename))
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

  string ReplayStream::status()
  {
    if (m_infile.is_open())
    {
      return "playing";
    }
    if (m_outfile.is_open())
    {
      return "recording";
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
      m_outfile.write((char *)&tStamp, sizeof(tStamp));
      uint16_t cnt = htons(packet->getSize()); // Sizeof the packet
      m_outfile.write((char *)&cnt, sizeof(cnt));
      m_outfile.write((char *)packet->getPacket(), packet->getSize()); // The packet itself (encoded)
      return true;
    }
    return false;
  }

  JamNetStuff::JamPacket *ReplayStream::getJamPacket()
  {
    return &m_packet;
  }

  JamNetStuff::JamPacket *ReplayStream::getPlayBackMix()
  {
    if (!m_infile.is_open())
    {
      return NULL;
    }
    // read audio from replay stream
    while (packetReady())
    {
      // feed the mixer with packets up till now...
      m_mixer.addData(&m_packet);
      readPacket();
    }

    // debug
    // if (++m_framecount % 375 == 0)
    // {
    //   m_mixer.dumpOut();
    // }

    uint64_t outFrameTime = 128 * 1000 / 48;
    m_delta += m_timer.getExpiredTime(); // add time since we did this last
    if (m_delta > outFrameTime)
    {
      // it's been over 128 samnples of time.  Let's make a packet
      m_mixer.getMix(m_outputs, 128);
      m_sendPacket.encodeAudio((const float **)m_outputs, 128);
      m_sendPacket.encodeHeader();
      m_delta -= outFrameTime;
      return &m_sendPacket;
    }

    return NULL;
  }
}