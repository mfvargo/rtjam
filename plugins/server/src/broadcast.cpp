#include <iostream>
#include <string.h>
#include "JamNetStuff.hpp"
#include <vector>
#include <thread>
#include <cmath>

std::vector<std::thread> roomThreads;

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE 960

void fifo_thread(short port, JamNetStuff::JamMixer *jamMixer)
{
  JamNetStuff::MicroTimer timer;
  float *outputs[NUM_OUTPUTS];
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    outputs[i] = new float[FIFO_FRAME_SIZE];
  }
  unsigned char pcmBuf[FIFO_FRAME_SIZE * sizeof(short)];
  char fifo[128];
  int fd_fifo;
  sprintf(fifo, "/home/pi/www/ice/%d", port);
  if ((fd_fifo = open(fifo, O_RDWR | O_NONBLOCK)) == -1)
  {
    fprintf(stderr, "fifo open: %s %s (%d)\n", fifo, strerror(errno), errno);
    return;
  }
  uint64_t delta = 0;
  uint64_t outFrameTime = FIFO_FRAME_SIZE * 1000 / 48;
  while (1)
  {
    std::this_thread::sleep_for(std::chrono::microseconds(outFrameTime - delta));
    delta += timer.getExpiredTime();
    if (delta > outFrameTime)
    {
      // fprintf(stderr, "%d\n", delta);
      // Pump out some data
      jamMixer->getMix(outputs, FIFO_FRAME_SIZE);
      for (int i = 0; i < FIFO_FRAME_SIZE; i++)
      {
        float float_in = outputs[0][i];
        // float float_in = sin((2.0 * 3.14 * i)/FIFO_FRAME_SIZE );
        short temp = float_in * 32768;
        memcpy(&pcmBuf[i * 2], &temp, 2);
      }
      if (write(fd_fifo, pcmBuf, FIFO_FRAME_SIZE * sizeof(short)) != FIFO_FRAME_SIZE * sizeof(short))
      {
        if (errno != 11)
        {
          fprintf(stderr, "fifo write: %s %s (%d)\n", fifo, strerror(errno), errno);
        }
      }
      delta -= outFrameTime;
      if (delta > outFrameTime)
      {
        // slip
        delta = 0;
      }
    }
  }
}

void packet_thread(short port)
{
  JamNetStuff::JamSocket jamSocket;
  JamNetStuff::JamMixer jamMixer;
  jamSocket.initServer(port);
  jamSocket.isActivated = true;
  // Max out the buffers on the mixer
  for (int i = 0; i < MAX_JAMMERS; i++)
  {
    jamMixer.setBufferSmoothness(i, 0.5);
  }
  // Start a thread to write the mix to the FIFO
  std::thread fifoThread = std::thread(fifo_thread, port, &jamMixer);
  // Loop and broadcast data
  while (1)
  {
    jamSocket.doPacket(&jamMixer);
  }
}

int main(int argc, char **argv)
{
  short port = 7891;
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  // Now wait for the threads to exit
  for (auto &element : roomThreads)
  {
    element.join();
  }
  return 0;
}