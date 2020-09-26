#include <iostream>
#include "../../common/JamNetStuff.hpp"

int main(int argc, char **argv) {

  JamNetStuff::JamSocket jamSocket;
  JamNetStuff::ChannelMap channelMap;

  std::cout << "Hello World!\n";

  // as the server, bind the socket to port 7891
  jamSocket.initServer();

  jamSocket.isActivated = true;

  // Loop and broadcast data
  while (1) {
    jamSocket.readPackets();
  }

  return 0;
}