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
#include "JamNetStuff.hpp"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    cerr << "sndSplit: missing paramaters" << endl;
    cerr << "usage:  sndSplit infile.raw" << endl;
    return EXIT_FAILURE;
  }
  // The replay object will reconstruct the mix
  JamNetStuff::ReplayStream *replay = new JamNetStuff::ReplayStream();
  int srate = 48000;
  uint64_t asOf = JamNetStuff::getMicroTime();
  uint64_t microFrameTime = 128 * 1000 / 48;

  if (replay->readOpen(argv[1]) != "playing")
  {
    cerr << "failed to open raw packet file: " << argv[1] << endl;
    return EXIT_FAILURE;
  }

  json metadata = json::parse(replay->getMetdata());
  cout << metadata.dump(2) << endl;
  cout << metadata.size() << endl;

  // Open the output files
  map<int, ofstream> clientMap;
  for (int i = 0; i < metadata.size(); i++)
  {
    cout << metadata[i][0] << ", " << metadata[i][1] << endl;
    string filename = "split-" + to_string(metadata[i][0]) + ".raw";
    clientMap.emplace(metadata[i][0], ofstream(filename, ios::out | ios::binary));
  }

  // write new metadata
  for (auto i = clientMap.begin(); i != clientMap.end(); i++)
  {
    json header = json::array();
    header.push_back({i->first, 0}); // latency metadata header
    string metadata(header.dump());
    uint16_t count = metadata.size() + 1; // null terminator
    count = htons(count);
    i->second.write((char *)&count, sizeof(count));
    i->second.write(metadata.c_str(), metadata.size() + 1);
  }
  int frameCount = 0;
  replay->m_debug = true;

  while (replay->readPacket())
  {
    frameCount++;
    uint64_t timeStamp = replay->getTimeStamp();
    JamNetStuff::JamPacket *pPacket = replay->getJamPacket();
    auto item = clientMap.find(pPacket->getClientIdFromPacket()); // is the clientId part of the map?
    if (item != clientMap.end())
    {
      uint64_t stamp = htobe64(replay->getTimeStamp());
      item->second.write((char *)&stamp, sizeof(stamp));
      uint16_t cnt = htons(pPacket->getSize());
      pPacket->encodeHeader();
      item->second.write((char *)&cnt, sizeof(cnt));
      item->second.write((char *)pPacket->getPacket(), pPacket->getSize());
    }
  }
  cout << "frames: " << frameCount << endl;
  return 0;
}