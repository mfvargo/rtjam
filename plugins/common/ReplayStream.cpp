#include "JamNetStuff.hpp"

namespace JamNetStuff
{
  ReplayStream::ReplayStream()
  {
    m_debug = false;
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
      m_infile.close();
    }
    m_infile.open(filename, ios::in | ios::binary);
    if (!m_infile.good())
    {
      return "error opening file";
    }
    // read the metadata off the top.
    uint16_t cnt;
    m_infile.read((char *)&cnt, sizeof(cnt));
    cnt = ntohs(cnt);
    if (cnt > 2048)
    {
      // corrumpt file?
      close();
      return "file seems corrupt";
    }
    char buffer[2048];
    m_infile.read(buffer, cnt);
    m_metadata = string(buffer);

    // now that we are here, let's load the first packet
    if (!readPacket())
    {
      // Could not read first packet.  close
      close();
      return "error reading file";
    }
    uint64_t now = getMicroTime();
    m_timeOffset = now - m_timeStamp;
    // for (int i = 0; i < MAX_JAMMERS; i++)
    // {
    //   m_mixer.setBufferSmoothness(i, 0.5);
    // }
    return "playing";
  }

  string ReplayStream::writeOpen(const char *filename, string metadata)
  {
    m_metadata = metadata;
    // Open the file
    if (m_outfile.is_open())
    {
      return "recording";
    }
    if (!m_outfile.open(filename))
    {
      return "error opening file";
    }
    uint16_t count = metadata.size() + 1; // null terminator
    count = htons(count);
    m_outfile.write((char *)&count, sizeof(count));
    m_outfile.write(metadata.c_str(), metadata.size() + 1);
    return "recording";
  }

  string ReplayStream::close()
  {
    m_metadata.clear();
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

  bool ReplayStream::packetReady(uint64_t asOf)
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
    return (asOf > m_timeStamp + m_timeOffset);
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
        //  m_packet.dumpPacket("replay: ");
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
    dump();
    // read audio from replay stream
    uint64_t asOf = getMicroTime();
    while (packetReady(asOf))
    {
      // feed the mixer with packets up till now...
      m_mixer.addData(&m_packet);
      readPacket();
    }

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

  float **ReplayStream::getMix(uint64_t asOf)
  {
    dump();
    while (packetReady(asOf))
    {
      // feed the mixer with packets up till now...
      m_mixer.addData(&m_packet);
      readPacket();
    }
    m_mixer.getMix(m_outputs, 128);
    return m_outputs;
  }

  void ReplayStream::dump()
  {
    if (m_debug && ++m_framecount % 375 == 0)
    {
      m_mixer.dumpOut();
    }
  }
}