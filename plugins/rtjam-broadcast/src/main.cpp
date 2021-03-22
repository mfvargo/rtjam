#include <iostream>
#include <string.h>
#include "JamNetStuff.hpp"
#include "Settings.hpp"
#include "RTJamNationApi.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>

using namespace std;

vector<std::thread> roomThreads;

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE 960

void packet_thread(short port) {
  JamNetStuff::JamSocket jamSocket;
  jamSocket.initServer(port);
  jamSocket.isActivated = true;
  // Loop and broadcast data
  while (1) {
    jamSocket.readAndBroadcast(NULL);
  }
}

int main(int argc, char **argv) {
  Settings settings;
  settings.loadFromFile();
  string urlBase = settings.getOrSetValue("rtjam-nation", std::string("rtjam-nation.basscleftech.com"));
  string token = settings.getOrSetValue("rtjam-broadcast-token", "");
  settings.saveToFile();
  RTJamNationApi api(urlBase);
  while (api.broadcastUnitPing(token) == false) {
    clog << "waiting to checkin with the nation" << endl;
    sleep(5);
  }

  for (short port = 7891; port < 7895; port++) {
    char roomName[100];
    sprintf(roomName, "Room: %d", port);
    api.activateRoom(token, roomName, port);
    roomThreads.push_back(std::thread(packet_thread, port));
  }
  // Now wait for the threads to exit
  for (auto & element : roomThreads) {
    element.join();
  }
  return 0;
}