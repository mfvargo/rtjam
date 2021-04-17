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

vector<thread> roomThreads;

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE 960

void packet_thread(short port, json room) {
  clog << room.dump(2) << endl;
  JamNetStuff::JamSocket jamSocket;
  jamSocket.initServer(port);
  jamSocket.isActivated = true;
  // Loop and broadcast data
  while (1) {
    int nBytes = jamSocket.doPacket(NULL);
  }
}

int main(int argc, char **argv) {
  Settings settings;
  settings.loadFromFile();
  string urlBase = settings.getOrSetValue("rtjam-nation", std::string("http://rtjam-nation.basscleftech.com/api/1/"));
  string token = "";
  settings.saveToFile();
  RTJamNationApi api(urlBase);
  
  while (token == "") {
    clog << "waiting to checkin with the nation" << endl;
    if (api.broadcastUnitDeviceRegister() && api.m_httpResponseCode == 200) {
      // get the token
      token = api.m_resultBody["broadcastUnit"]["token"];
      clog << "got a token: " << token << endl;
    }
    sleep(5);
  }

  for (short port = 7891; port < 7894; port++) {
    char roomName[100];
    sprintf(roomName, "Room: %d", port);
    api.activateRoom(token, roomName, port);
    // Start up the room thread with their attributes
    roomThreads.push_back(std::thread(packet_thread, port, api.m_resultBody["room"]));
  }

  while(1) {
    sleep(10);
    if (token == "") {
      // We don't have a token.  Re-register the device.
      if (api.broadcastUnitDeviceRegister() && api.m_httpResponseCode == 200) {
        // get the token
        token = api.m_resultBody["broadcastUnit"]["token"];
        clog << "got a new token: " << token << endl;
        // TODO: shutdown the rooms?
      }
    } else {
      if (!api.broadcastUnitPing(token) || api.m_httpResponseCode != 200) {
          // Something is wrong with this token
          token = "";
      };
    }
  }
  // Now wait for the threads to exit
  for (auto & element : roomThreads) {
    element.join();
  }
  return 0;
}