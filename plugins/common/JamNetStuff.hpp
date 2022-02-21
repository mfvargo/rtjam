/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 */

#ifndef JAM_NET_STUFF_HPP_INCLUDED
#define JAM_NET_STUFF_HPP_INCLUDED

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "jamrtime.h"
#include <mutex>
#include <vector>
#include <map>
#include "MetroNome.hpp"
#include "PeakDetector.hpp"

#define EMPTY_SLOT 40000

namespace JamNetStuff
{
  void encodeJamMessage(struct JamMessage *packet);
  void decodeJamMessage(struct JamMessage *packet);
  uint64_t getMicroTime();

  // StopWatch in microseconds
  class MicroTimer
  {
  public:
    MicroTimer();
    void reset();
    uint64_t getExpiredTime();
    uint64_t getTimeFromStart();
    void setTimerInterval(uint64_t interval)
    {
      m_interval = interval;
    }
    uint64_t getTimerInterval()
    {
      return m_interval;
    }
    bool isExpired();

  private:
    uint64_t m_lastTime;
    uint64_t m_startTime;
    uint64_t m_interval;
  };

  // Class to calculate statistics on Stuff using cheap and easy avg func
  class StreamTimeStats
  {
  public:
    StreamTimeStats();

    float mean;
    float sigma;
    float peak;
    bool started;
    float windowSize;

    void clear();
    void addSample(float sample);
  };

  // Class to map network channels into local channels. Used on the client side
  // to track who is in the room.
  class ChannelMap
  {
  public:
    ChannelMap();
    int getChannel(uint32_t clientId);
    void setMyId(uint32_t Id);
    void dumpOut();
    void clear();
    uint32_t getClientId(int idx) { return channels[idx].clientId; };
    void pruneStaleChannels(time_t now, int startAt = 1);
    void getClientIds(uint32_t *ids)
    {
      for (int i = 0; i < MAX_JAMMERS; i++)
      {
        ids[i] = channels[i].clientId;
      }
    };

  private:
    struct Channel
    {
      uint32_t clientId;
      time_t KeepAlive;
    };
    uint32_t myId;
    Channel channels[MAX_JAMMERS];
  };

  class Player
  {
  public:
    uint32_t clientId;
    uint64_t KeepAlive;
    bool bPinging;
    sockaddr_in Address;
    StreamTimeStats networkTime;
  };

  // Class to hold the list of players currently in the room on the server side.
  class PlayerList
  {
  public:
    PlayerList();
    void setAllowedClientIds(::std::vector<unsigned> &ids);
    bool isAllowed(unsigned clientId);
    int updateChannel(unsigned clientId, sockaddr_in *addr, uint64_t pingTime);
    int numPlayers();
    Player get(int i);
    void startPing();
    void Prune();
    void dump(::std::string msg);
    std::map<unsigned, float> getLatency();

  private:
    ::std::vector<unsigned> m_allowedClientIds;
    ::std::vector<Player> m_players;
    int m_roomSize;
  };

#define JITTER_SAMPLES 96000

  class JitterBuffer
  {
  public:
    JitterBuffer();

    void flush(int initialDepth = 0);
    int depth();
    void putIn(const float *buffer, int frames, uint32_t seqNo);
    void getOut(float *buffer, int frames);
    void dumpOut();
    void setSmoothness(float smooth);
    float getAvgDepth();

  protected:
    PeakDetector m_depthFilter;
    float myBuffer[JITTER_SAMPLES];
    float lastFrame[MAX_FRAME_SIZE];
    int readIdx;
    int writeIdx;
    int maxDepth;
    unsigned targetDepth;
    int numOverruns;
    int numUnderruns;
    int numPuts;
    float nSigma;
    unsigned numGets;
    uint32_t lastSequence;
    int numDropped;
    int numDups;
    StreamTimeStats bufferStats;
    bool m_filling;
    int m_fillDepth;

    void copySamples(float *dst, const float *src, int count);
  };

  // Class to represent a JamPacket
  class JamPacket
  {
  public:
    JamPacket();

    // Take a frame of floats and encode two channels into 16 bit PCM data
    void encodeAudio(const float **inputs, int frames);
    // Encode just the header of the jamMessage
    void encodeHeader();
    // Decode just the header of the jamMessage
    int decodeHeader(int nBytes);
    // Decode the jamMessage into two arrays of floats (one for each subchannel)
    int decodeJamBuffer(float **outputs);

    void *getPacket();
    int getSize(bool headerOnly = false);
    bool isMe();
    uint32_t getSequenceNo();
    void dumpPacket(const char *);
    int getChannel();
    void checkChannelTimeouts() { channelMap.pruneStaleChannels(time(NULL)); };
    void setIsClient(bool val) { isClient = val; };

    void clearChannelMap();
    void setServerChannel(int channel);
    uint8_t getServerChannel() { return jamMessage.Channel; };
    void setBeatCount(char beat) { jamMessage.Beat = beat; };
    char getBeatCount() { return jamMessage.Beat; };
    uint64_t getServerTime() { return jamMessage.ServerTime; };
    void setServerTime(uint64_t serverTime) { jamMessage.ServerTime = serverTime; };
    void getClientIds(uint32_t *ids) { channelMap.getClientIds(ids); };
    void setClientId(uint32_t id)
    {
      clientId = id;
      clearChannelMap();
    };
    uint32_t getClientId() { return clientId; };
    uint32_t getClientIdFromPacket() { return jamMessage.ClientId; }

  private:
    bool isClient;
    // Convert one channel of floats to 16 bit PCM
    void encodeJamBuffer(unsigned char *, const float *, int);
    bool validPacket(int nBytes);
    JamMessage jamMessage;
    uint32_t clientId;
    int bufferSize;
    uint32_t sequenceNo;
    JAM_SAMPLE_RATES sampleRate;
    ChannelMap channelMap;
    int numSamples;
  };

#define MIX_CHANNELS MAX_JAMMERS * 2

  class JamMixer
  {
  public:
    JamMixer();
    /* print out some stats */
    void dumpOut();
    // reset to default values
    void reset();
    // clear other player volumes (used when changing rooms)
    void clearPlayerVolumes();
    /* get some data for the output */
    void getMix(float **outputs, uint32_t frames);
    /* write a jamPacket */
    void addData(JamPacket *packet);
    /* write local monitoring data */
    void addLocalMonitor(const float **inputs, uint32_t frames);
    /* set the metronome config */
    void setMetronomeConfig(int /*soundType*/, float volume)
    {
      m_metronome.setVolume(volume);
      // m_metronome.init((MetroNome::SoundType)soundType, volume);
    }

    float masterVol;
    float gains[MIX_CHANNELS];
    float fades[MIX_CHANNELS];
    float channelLevels[MIX_CHANNELS];
    float peakLevels[MIX_CHANNELS];
    float bufferDepths[MIX_CHANNELS];
    float masterLevel;
    float masterPeak;

    void setBufferSmoothness(int channel, float smooth);
    char getBeat() { return beat; };

  protected:
    JitterBuffer jitterBuffers[MIX_CHANNELS];
    float mixBuffers[MIX_CHANNELS][MAX_FRAME_SIZE];
    StreamTimeStats levelStats[MIX_CHANNELS];
    StreamTimeStats masterStats;

  private:
    char beat;
    float cBuf0[MAX_FRAME_SIZE];
    float cBuf1[MAX_FRAME_SIZE];
    float *conversionBuf[2];
    MetroNome m_metronome;
    uint32_t m_seqNo;
  };

  // Class to read and write packets streams to a file
  class ReplayStream
  {
  public:
    ReplayStream();
    ~ReplayStream();
    string readOpen(const char *filename);
    string writeOpen(const char *filename);
    string close();
    bool writePacket(JamNetStuff::JamPacket *packet);
    bool readPacket();
    bool packetReady();
    JamNetStuff::JamPacket *getJamPacket();
    JamPacket *getPlayBackMix();

  private:
    JamNetStuff::JamPacket m_packet;
    JamNetStuff::JamPacket m_sendPacket;
    JamNetStuff::JamMixer m_mixer;
    JamNetStuff::MicroTimer m_timer;
    ifstream m_infile;
    ofstream m_outfile;
    int64_t m_timeOffset;
    uint64_t m_timeStamp;
    uint64_t m_delta;
    int m_framecount;
    float *m_outputs[MIX_CHANNELS + 2]; // each channel plus 2 for the overall mix
  };

  class JamSocket
  {
  public:
    JamSocket();
    int sendPacket(const float **buffer, int frames);
    void readPackets(JamMixer *);
    void sendDataToRoomMembers(JamMixer *);
    bool isActivated;

    void initServer(short port);
    void initClient(const char *servername, int port, uint32_t clientId);
    void disconnect()
    {
      isActivated = false;
      m_packet.clearChannelMap();
    }
    void setTempo(int newTempo) { m_tempo = newTempo; };
    int getTempo() { return m_tempo; };
    std::map<unsigned, float> getLatency() { return m_playerList.getLatency(); };
    void getClientIds(uint32_t *ids) { m_packet.getClientIds(ids); };

    string recordRoom();
    string stopAudio();
    string playAudio();

  private:
    PlayerList m_playerList;
    JamPacket m_packet;
    int jamSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in senderAddr;
    socklen_t addr_size;
    int readData();
    int sendData(struct sockaddr_in *to_addr, bool headerOnly, JamPacket *pPacket);
    uint64_t beatCount;
    uint64_t lastPingTime;
    int m_tempo;
    uint64_t m_tempoStart;
    bool m_pinging;
    ReplayStream m_capture;
    short m_port;
  };
};

#endif // JAM_NET_STUFF_HPP_INCLUDED
