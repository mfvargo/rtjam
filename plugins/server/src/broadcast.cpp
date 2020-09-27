#include <iostream>
#include <string.h>
#include "../../common/JamNetStuff.hpp"

int main(int argc, char **argv) {

  JamNetStuff::JamSocket jamSocket;
  short port = 7891;

  if (argc > 2 && strcmp(argv[1],"--port") == 0) {
    port = atoi(argv[2]);
    printf("port = %s\n", argv[2]);
  }


  std::cout << "Hello World!\n";

  // as the server, bind the socket to port 7891
  jamSocket.initServer(port);

  jamSocket.isActivated = true;

  // Loop and broadcast data
  while (1) {
    jamSocket.readAndBroadcast();
  }

  return 0;
}