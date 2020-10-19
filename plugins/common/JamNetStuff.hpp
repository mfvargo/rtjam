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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "../common/jamrtime.h"

#define EMPTY_SLOT 40000

namespace JamNetStuff
{
  void encodeJamMessage(struct JamMessage* packet);
  void decodeJamMessage(struct JamMessage* packet);
  uint64_t getMicroTime();

  // Class to measure microsecond timers
  class MicroTimer {
    public:
      MicroTimer();
      uint64_t getExpiredTime();
      uint64_t getTimeFromStart();
    
    private:
      uint64_t lastTime;
      uint64_t startTime;
    
  };

  // Class to calculate statistics on Stuff using cheap and easy avg func
  class StreamTimeStats {
    public:
      StreamTimeStats();

      float mean;
      float dxdt;
      float sigma;
      float sigmaPrime;
      bool started;
      float windowSize;

      void clear();
      void addSample(float sample);
  };

  // Class to map network channels into local channels
  class ChannelMap {
    public:
      ChannelMap();
      int getChannel(uint32_t clientId, sockaddr_in *addr = NULL);
      void setMyId(uint32_t Id);
      void dumpOut();
      void clear();
      uint32_t getClientId(int idx) { return channels[idx].clientId; };
      void getClientAddr(int idx, sockaddr_in *addr);
      void pruneStaleChannels(time_t now, int startAt = 1);

    private:
      struct Channel {
        uint32_t clientId;
        time_t KeepAlive;
        sockaddr_in Address;
      };
      uint32_t myId;
      Channel channels[MAX_JAMMERS];
      void makeIpString(unsigned long s_addr, char* ipString);
  };


  #define JITTER_SAMPLES 96000

  class JitterBuffer {
    public:
      JitterBuffer();

      void flush();
      int depth();
      void putIn(const float* buffer, int frames, uint32_t seqNo);
      void getOut(float* buffer, int frames);
      void dumpOut();
      void setSmoothness(float smooth);
      float getAvgDepth();
    
    protected:

      struct JamMessage myPacket;
      float myBuffer[JITTER_SAMPLES];
      float lastFrame[MAX_FRAME_SIZE];
      int readIdx;
      int writeIdx;
      int maxDepth;
      int targetDepth;
      int numOverruns;
      int numUnderruns;
      int numPuts;
      int lastSequence;
      int numDropped;
      StreamTimeStats bufferStats;
      StreamTimeStats overrunStats;
      StreamTimeStats underrunStats;

      void copySamples(float* dst, const float* src, int count);
  };

  // Class to represent a JamPacket
  class JamPacket {
    public:
      JamPacket();

      // Take a frame of floats and encode two channels into 16 bit PCM data
      void encodeAudio(const float** inputs, int frames);
      // Encode just the header of the jamMessage
      void encodeHeader();
      // Decode just the header of the jamMessage
      int decodeHeader(int nBytes);
      // Decode the jamMessage into two arrays of floats (one for each subchannel)
      int decodeJamBuffer(float** outputs);

      void* getPacket();
      int getSize();
      bool isMe();
      uint32_t getSequenceNo();
      void dumpPacket(const char*);
      int getChannel();
      void setIsClient(bool val) { isClient = val; };

      void clearChannelMap() {
        sequenceNo = 0;
        channelMap.clear();
      };
      void setServerChannel(int channel);
      uint8_t getServerChannel() { return jamMessage.Channel; };
      void setBeatCount(char beat) { jamMessage.Beat = beat; };
      char getBeatCount() { return jamMessage.Beat; };
      uint64_t getServerTime() { return jamMessage.ServerTime; };

    private:
      bool isClient;
      // Convert one channel of floats to 16 bit PCM
      void encodeJamBuffer(unsigned char*, const float*, int);
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

  class JamMixer {
    public:
      JamMixer();
      /* print out some stats */
      void dumpOut();
      /* get some data for the output */
      void getMix(float** outputs, uint32_t frames);
      /* write a jamPacket */
      void addData(JamPacket* packet);
      /* write local monitoring data */
      void addLocalMonitor(const float** inputs, uint32_t frames);

      float masterVol;
      float gains[MIX_CHANNELS];
      float channelLevels[MIX_CHANNELS];
      float bufferDepths[MIX_CHANNELS];
      float masterLevel;

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
      float* conversionBuf[2];
  };

  class JamSocket {
    public:
      JamSocket();
      int sendPacket(const float** buffer, int frames);
      int readPackets(JamMixer*);
      int readAndBroadcast(JamMixer*);
      bool isActivated;

      void initServer(short port);
      void initClient(const char* servername, int port);
      void channelDump() { channelMap.dumpOut(); };
      void setTempo(int newTempo) { tempo = newTempo; };
    
    private:
      ChannelMap channelMap;
      JamPacket packet;
      int jamSocket;
      struct sockaddr_in serverAddr;
      struct sockaddr_in senderAddr;
      socklen_t addr_size;
      int readData();
      int sendData(struct sockaddr_in* to_addr);
      uint64_t beatCount;
      uint64_t lastClickTime;
      int tempo;
  };
};

#endif // JAM_NET_STUFF_HPP_INCLUDED
