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

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    cerr << "sndExport: missing paramaters" << endl;
    cerr << "usage:  sndExport infile.raw outfile.wav" << endl;
    return EXIT_FAILURE;
  }
  JamNetStuff::ReplayStream *replay = new JamNetStuff::ReplayStream();
  SndfileHandle file;
  int srate = 48000;
  uint64_t asOf = JamNetStuff::getMicroTime();
  uint64_t microFrameTime = 128 * 1000 / 48;
  float **mix;
  float buffer[128 * 16]; // up to 16 channels

  if (replay->readOpen(argv[1]) != "playing")
  {
    cerr << "failed to open raw packet file: " << argv[1] << endl;
    return EXIT_FAILURE;
  }

  json metadata = json::parse(replay->getMetdata());
  cout << metadata.dump(2) << endl;
  cout << metadata.size() << endl;
  int channels = metadata.size() * 2;

  if (!(file = SndfileHandle(argv[2], SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, channels, srate)))
  {
    cerr << "Cannot create file " << argv[2] << endl;
  }

  while (replay->status() == "playing")
  {
    asOf += microFrameTime;
    mix = replay->getMix(asOf);
    if (mix)
    {
      float *bufptr = buffer;
      // interleave buffers
      for (int i = 0; i < 128; i++)
      {
        // This starts at 4 because chan 0/1 are the mix, 2/3 are local playback, 4+ from the stream.
        for (int chan = 4; chan < channels + 4; chan++)
        {
          *bufptr++ = mix[chan][i];
        }
      }
      file.write(buffer, 128 * channels);
    }
  }
  return 0;
}