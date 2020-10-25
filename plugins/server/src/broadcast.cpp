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

  // Space them so the printfs dont interleave
  std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 68));

  // Loop and broadcast data
  time_t now = time(NULL);
  time_t lastDump = time(NULL);
  while (1) {
    jamSocket.readAndBroadcast(&jamMixer);
    now = time(NULL);
    if (now - lastDump > 3) {
      // printf("port %d: ", port);
      // jamSocket.channelDump();
      lastDump = now;
    }
  }
}

int main(int argc, char **argv) {
  short port = 7891;
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  roomThreads.push_back(std::thread(packet_thread, port++));
  // Now wait for the thread to exit
  for (auto & element : roomThreads) {
    element.join();
  }
  return 0;
}