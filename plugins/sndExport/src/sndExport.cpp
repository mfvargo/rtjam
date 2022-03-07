/*
 ** Copyright (C) 2007-2011 Erik de Castro Lopo <erikd@mega-nerd.com>
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <iostream>
#include <string>
#include <sndfile.hh>
#include "JamNetStuff.hpp"
#include <filesystem>

using namespace std;
namespace fs = filesystem;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "sndExport: missing paramater" << endl;
    cerr << "usage:  sndExport infile.raw" << endl;
    return EXIT_FAILURE;
  }

  string filename(argv[1]);

  // The replay object will reconstruct the mix
  JamNetStuff::ReplayStream *replay = new JamNetStuff::ReplayStream();
  // File handle for the output
  SndfileHandle file;
  int srate = 48000;
  uint64_t asOf = JamNetStuff::getMicroTime();
  uint64_t microFrameTime = 128 * 1000 / 48;
  float **mix;
  float buffer[128 * 16]; // up to 16 channels

  if (replay->readOpen(filename.c_str()) != "playing")
  {
    cerr << "failed to open raw packet file: " << argv[1] << endl;
    return EXIT_FAILURE;
  }

  json metadata = json::parse(replay->getMetdata());
  cout << metadata.dump(2) << endl;
  cout << metadata.size() << endl;
  int channels = metadata.size() * 2; // Note the times 2 here.  Each latency record in the metadata has two channels.

  // map the latency values so we can use them to delay adjust individual channels.
  map<int, float> latencyMap;
  for (int i = 0; i < metadata.size(); i++)
  {
    cout << metadata[i][0] << ", " << metadata[i][1] << endl;
    latencyMap.emplace(metadata[i][0], metadata[i][1]);
  }

  // Open the output file
  string wavfile = fs::path(filename).stem();
  wavfile += ".wav";
  if (!(file = SndfileHandle(wavfile.c_str(), SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, channels, srate)))
  {
    cerr << "Cannot create file " << wavfile << endl;
  }

  // The delayAdjust are just queues of floats.  They all start out even, but after the channelmap
  // has been worked out, they get different values stuffed in to compensate for latencies.
  queue<float> delayAdjust[channels];

  int frameCount = 0;

  replay->m_debug = true;
  // This is where I would adjust queue depths based on latency
  while (replay->status() == "playing")
  {
    frameCount++;
    if (frameCount == 200)
    {
      // so at this point the channelmap should be set.
      // get the channel ids (for the map), figure out how much delay to insert in each delayAdjuster
      uint32_t ids[MAX_JAMMERS];
      replay->getClientIds(ids);
      for (int i = 0; i < MAX_JAMMERS; i++)
      {
        auto item = latencyMap.find(ids[i]); // is the clientId part of the map?
        if (item != latencyMap.end())
        {
          // We have the latency for this channel.
          int addCount = 48 * (100 - item->second); // every msec is 48 samples.
          cout << "channel " << i << " with id " << ids[i] << " has latency " << item->second;
          cout << " adding " << addCount << "samples" << endl;
          for (int k = 0; k < addCount; k++)
          {
            // This is (i-1) cause channel 0 is always the local playback which does not apply in this scenario
            // Also note that this assumes the channel map is packed tight (which it should be)
            delayAdjust[2 * (i - 1)].push(0.0);
            delayAdjust[2 * (i - 1) + 1].push(0.0);
          }
        }
      }
    }
    asOf += microFrameTime;
    mix = replay->getMix(asOf);
    if (mix)
    {
      float *bufptr = buffer;
      // put mixes into delay lines
      for (int chan = 0; chan < channels; chan++) // each channel
      {
        for (int i = 0; i < 128; i++) // each sample
        {
          delayAdjust[chan].push(mix[chan + 4][i]);
        }
      }
      // pull samples and interleave from delay lines (sample by sample interleave)
      for (int i = 0; i < 128; i++) // each sample
      {
        // This starts at 4 because chan 0/1 are the mix, 2/3 are local playback, 4+ from the stream.
        for (int chan = 0; chan < channels; chan++) // each channel
        {
          *bufptr++ = delayAdjust[chan].front();
          delayAdjust[chan].pop();
        }
      }

      file.write(buffer, 128 * channels);
    }
  }
  // reopen the file
  if (replay->readOpen(argv[1]) != "playing")
  {
    cerr << "failed to open raw packet file: " << argv[1] << endl;
    return EXIT_FAILURE;
  }

  string statfile = fs::path(filename).stem();
  statfile += ".csv";

  ofstream csvFile(statfile);
  csvFile << "clientId,timestamp,seq" << endl;
  while (replay->readPacket())
  {
    uint64_t timeStamp = replay->getTimeStamp();
    JamMessage *pMsg = (JamMessage *)replay->getJamPacket()->getPacket();
    csvFile << pMsg->ClientId << "," << timeStamp << "," << pMsg->SequenceNumber << endl;
  }
  return 0;
}