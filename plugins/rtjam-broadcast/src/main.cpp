#include <iostream>
#include <string.h>
#include "JamNetStuff.hpp"
#include "Settings.hpp"
#include "RTJamNationApi.hpp"
#include "PlayerList.hpp"
#include <vector>
#include <thread>
#include <mutex>
#include <cmath>

using namespace std;

vector<std::thread> roomThreads;

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE 960

void packet_thread(short port, string token) {
  JamNetStuff::JamSocket jamSocket;
  jamSocket.initServer(port);
  jamSocket.isActivated = true;
  // Loop and broadcast data
  while (1) {
    jamSocket.readAndBroadcast(NULL);
  }
}

int main(int argc, char **argv) {
  PlayerList players;
  Settings settings;
  settings.loadFromFile();
  string urlBase = settings.getOrSetValue("rtjam-nation", std::string("rtjam-nation.basscleftech.com"));
  string myToken = settings.getOrSetValue("rtjam-broadcast-token", "");
  settings.saveToFile();
  RTJamNationApi api(urlBase);

  string roomToken;
  
  while (api.broadcastUnitPing(myToken) == false) {
    clog << "waiting to checkin with the nation" << endl;
    sleep(5);
  }

  for (short port = 7891; port < 7895; port++) {
    char roomName[100];
    sprintf(roomName, "Room: %d", port);
    api.activateRoom(myToken, roomName, port);
    roomToken = api.m_resultBody["room"]["token"];
    api.playerList(roomToken);
    clog << api.m_resultBody.dump(2);
    // players.load(roomToken);
    // Start up the room thread with their port and their token
    roomThreads.push_back(std::thread(packet_thread, port, roomToken));
  }

  while(1) {
    sleep(10);
    api.broadcastUnitPing(myToken);
    api.playerList(roomToken);
    clog << api.m_resultBody.dump(2);
  }
  // Now wait for the threads to exit
  for (auto & element : roomThreads) {
    element.join();
  }
  return 0;
}