#include <iostream>
#include <string.h>
#include "../../common/JamNetStuff.hpp"
#include <vector>
#include <thread>
#include <mutex>

std::vector<std::thread> roomThreads;

void packet_thread(short port) {
  JamNetStuff::JamSocket jamSocket;
  JamNetStuff::JamMixer jamMixer;
  jamSocket.initServer(port);
  jamSocket.isActivated = true;
  float* outputs[12];
  for (int i=0; i<12; i++) {
    outputs[i] = new float[1024];
  }
  int16_t pcmBuf[1024];
  char fifo[128];
  int fd_fifo;
  sprintf(fifo, "/home/pi/www/ice/%d", port);
  if ((fd_fifo = open(fifo, O_RDWR | O_NONBLOCK )) == -1) {
    fprintf(stderr, "fifo open: %s %s (%d)\n", fifo, strerror(errno), errno);
    return;
  }
  // Loop and broadcast data
  uint64_t lastPump = JamNetStuff::getMicroTime();
  uint64_t delta = 0;
  while (1) {
    jamSocket.readAndBroadcast(&jamMixer);
    delta = JamNetStuff::getMicroTime() - lastPump;
    if ( delta > 20000) {
      // printf("%d: %ld\n", port, delta);
      // Pump out some data
      jamMixer.getMix(outputs, 960);
      for (int i=0; i<960; i++) {
        pcmBuf[i] = (short) (outputs[0][i] * 32766);
      }
      if (write (fd_fifo, pcmBuf, 960 * sizeof(int16_t)) != 960*sizeof(int16_t)) {
        if (errno != 11) {
          fprintf(stderr, "fifo write: %s %s (%d)\n", fifo, strerror(errno), errno);
        }
      }
      lastPump += 20000;
      if (delta > 40000) {
        // slipped, catch up
        lastPump = JamNetStuff::getMicroTime();
      }
    }
  }
}

int main(int argc, char **argv) {
  short port = 7891;
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  // Now wait for the threads to exit
  for (auto & element : roomThreads) {
    element.join();
  }
  return 0;
}