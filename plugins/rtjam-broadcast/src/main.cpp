#include <iostream>
#include <string.h>
#include "JamNetStuff.hpp"
#include "Settings.hpp"
#include "RTJamNationApi.hpp"
#include "RoomChatRobot.hpp"
#include <vector>
#include <thread>
#include <cmath>
#include <sys/stat.h>

using namespace std;

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE 960
#define MAX_ROOMS 3

vector<thread> roomThreads;
string roomTokens[MAX_ROOMS];

void fifo_thread(short port, JamNetStuff::JamMixer *jamMixer)
{
  JamNetStuff::MicroTimer timer;
  float *outputs[NUM_OUTPUTS];
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    outputs[i] = new float[FIFO_FRAME_SIZE];
  }
  unsigned char pcmBuf[FIFO_FRAME_SIZE * sizeof(short) * 2]; // 2 channels of shorts
  char fifo[128];
  int fd_fifo;
  sprintf(fifo, "/home/pi/rtjam/ice_%d", port);
  if (mkfifo(fifo, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1)
  {
    // Check if the file already exists, then just continue
    if (errno != EEXIST)
    {
      fprintf(stderr, "fifo create: %s %s (%d)\n", fifo, strerror(errno), errno);
    }
  }
  // Open or create the FIFO
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
      int bufOffset = 0;
      short sampleVal = 0;
      for (int i = 0; i < FIFO_FRAME_SIZE; i++)
      {
        // Convert the float to an int
        sampleVal = outputs[0][i] * 32768;
        // copy to the pcmBuf
        memcpy(&pcmBuf[bufOffset], &sampleVal, sizeof(short));
        bufOffset += sizeof(short);
        // Convert the float to an int
        sampleVal = outputs[1][i] * 32768;
        // copy to the pcmBuf
        memcpy(&pcmBuf[bufOffset], &sampleVal, sizeof(short));
        bufOffset += sizeof(short);
      }
      if (write(fd_fifo, pcmBuf, FIFO_FRAME_SIZE * sizeof(short) * 2) != FIFO_FRAME_SIZE * sizeof(short) * 2)
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

using easywsclient::WebSocket;
string bcastToken = "";

void websocket_thread(JamNetStuff::JamSocket *pJamSocket, int roomIdx)
{
  RoomChatRobot robot;
  while (true)
  {
    if (bcastToken != "" && roomTokens[roomIdx] != "")
    {
      robot.init("ws://rtjam-nation.basscleftech.com/primus", roomTokens[roomIdx], pJamSocket);
      robot.readMessages();
      cout << "Room lost" << endl;
      cout << "Bcast token: " << bcastToken << endl;
    }
    sleep(5);
  }
}

void packet_thread(short port, int roomIdx)
{
  JamNetStuff::JamSocket *pJamSocket = new JamNetStuff::JamSocket();
  JamNetStuff::JamMixer *pJamMixer = new JamNetStuff::JamMixer();
  pJamSocket->initServer(port);
  pJamSocket->isActivated = true;
  // Max out the buffers on the mixer
  for (int i = 0; i < MAX_JAMMERS; i++)
  {
    pJamMixer->setBufferSmoothness(i, 0.5);
  }
  // Start a thread to write the mix to the FIFO
  std::thread fifoThread = std::thread(fifo_thread, port, pJamMixer);

  // Start a thread to connect each room to a server chat room
  std::thread websocketThread = std::thread(websocket_thread, pJamSocket, roomIdx);

  // Loop and broadcast data
  while (1)
  {
    pJamSocket->sendDataToRoomMembers(pJamMixer);
    // pJamSocket->sendDataToRoomMembers(NULL);
  }
}

int main(int argc, char **argv)
{
  Settings settings;
  settings.saveVersionFile();
  settings.loadFromFile();
  string urlBase = settings.getOrSetValue("rtjam-nation", std::string("http://rtjam-nation.basscleftech.com/api/1/"));
  int startPort = settings.getOrSetValue("start-port", 7891);
  int roomCount = settings.getOrSetValue("room-count", 1);
  string broadcastUnitName = "";
  settings.saveToFile();
  RTJamNationApi api(urlBase);

  bool bFirstTime = true;

  while (1)
  {
    if (bcastToken == "")
    {
      // We don't have a token.  Re-register the device.
      if (api.broadcastUnitDeviceRegister() && api.m_httpResponseCode == 200)
      {
        // get the token
        bcastToken = api.m_resultBody["broadcastUnit"]["token"];
        broadcastUnitName = api.m_resultBody["broadcastUnit"]["name"];
        clog << "got a new token: " << bcastToken << endl;
        for (short port = startPort; port < startPort + roomCount; port++)
        {
          char roomName[100];
          sprintf(roomName, "%s:%d", broadcastUnitName.c_str(), port);
          api.activateRoom(bcastToken, roomName, port);
          roomTokens[port - startPort] = api.m_resultBody["room"]["token"];
          if (bFirstTime)
          {
            roomThreads.push_back(std::thread(packet_thread, port, port - startPort));
          }
        }
        bFirstTime = false;
      }
    }
    else
    {
      if (!api.broadcastUnitPing(bcastToken) || api.m_httpResponseCode != 200)
      {
        // Something is wrong with this token
        bcastToken = "";
        for (int i = 0; i < MAX_ROOMS; i++)
        {
          roomTokens[i] = "";
        }
      };
    }
    sleep(10);
  }
  // Now wait for the threads to exit
  for (auto &element : roomThreads)
  {
    element.join();
  }
  return 0;
}