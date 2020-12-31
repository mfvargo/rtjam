#include "ALSADevices.hpp"
#include "constants.h"
#include "JamNetStuff.hpp"
#include <iostream>
#include <thread>
#include <vector>

ALSACaptureDevice microphone("default", SAMPLING_RATE, CHANNELS, FRAMES_PER_PERIOD, FORMAT);
ALSAPlaybackDevice speaker("default", SAMPLING_RATE, CHANNELS, FRAMES_PER_PERIOD, FORMAT);
JamNetStuff::JamSocket jamSocket;
JamNetStuff::JamMixer jamMixer;

void read_audio() {
  JamNetStuff::MicroTimer timer;
  char* buffer = microphone.allocate_buffer();
  float* inputs[CHANNELS];
  for (int i=0; i<CHANNELS; i++) {
    inputs[i] = new float[FRAMES_PER_PERIOD];
  }

  // This is where we will read the microphone and push it to the network
  microphone.open();
  unsigned int frames_captured = 0;
  do{
    frames_captured = microphone.capture_into_buffer(buffer, FRAMES_PER_PERIOD);
    for(int i=0; i<FRAMES_PER_PERIOD; i++) {
      inputs[0][i] = ((float *) buffer)[i*2];
      inputs[1][i] = ((float *) buffer)[i*2+1];
    }
    fprintf(stderr, "frames: %d, time is: %lu\n", frames_captured, timer.getExpiredTime());
    jamMixer.addLocalMonitor((const float**) inputs, frames_captured);
    jamSocket.sendPacket((const float**)inputs, frames_captured);
    jamSocket.readPackets(&jamMixer);
    // std::this_thread::sleep_for(std::chrono::microseconds(1333));
  }while(1);
  microphone.close();
}

#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2
#define FIFO_FRAME_SIZE FRAMES_PER_PERIOD

void write_audio() {
  // this is where we will pace our code to write to the output
  char* buffer = speaker.allocate_buffer();
  speaker.open();
  JamNetStuff::MicroTimer timer;
  float* outputs[NUM_OUTPUTS];
  for (int i=0; i<NUM_OUTPUTS; i++) {
    outputs[i] = new float[FIFO_FRAME_SIZE];
  }
  uint64_t outFrameTime = FIFO_FRAME_SIZE * 1000 / 48;
  uint64_t delta = outFrameTime + 1;
  while(1) {
    while (delta > outFrameTime) {
      // fprintf(stderr, "%ld \n", speaker.space_available());
      // Pump out some data
      jamMixer.getMix(outputs, FIFO_FRAME_SIZE);
      for(int i=0; i<FRAMES_PER_PERIOD; i++) {
        ((float *) buffer)[i*2] = outputs[0][i];
        ((float *) buffer)[i*2+1] = outputs[1][i];
      }
      if (speaker.play_from_buffer(buffer, FIFO_FRAME_SIZE) == -EPIPE) {
        jamMixer.getMix(outputs, FIFO_FRAME_SIZE);
        speaker.play_from_buffer(buffer, FIFO_FRAME_SIZE);
      }
      delta -= outFrameTime;
      if (delta > outFrameTime) {
        // slip 
        // delta = 0;
        fprintf(stderr, "slip %lu\n", delta);
      }
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    delta += timer.getExpiredTime();
  }
  speaker.close();
}

int main() {
  jamMixer.reset();
  jamSocket.isActivated = true;
  jamSocket.initClient("music.basscleftech.com", 7891, 2334);
  std::vector<std::thread> audioThreads;
  audioThreads.push_back(std::thread(write_audio));
  audioThreads.push_back(std::thread(read_audio));
  for (auto & element : audioThreads) {
    element.join();
  }

  return 0;
}

// int main() {
//   microphone.open();
//   speaker.open();
//   char* buffer = microphone.allocate_buffer();
//   unsigned int frames_captured, frames_played;

//   do{
//     frames_captured = microphone.capture_into_buffer(buffer, FRAMES_PER_PERIOD);
//     frames_played = speaker.play_from_buffer(buffer, FRAMES_PER_PERIOD);
//     std::cout << "Captured,Played ---> " << frames_captured << "," << frames_played << std::endl;
//   }while(1);

//   microphone.close();
//   speaker.close();
//   return 0;
// }