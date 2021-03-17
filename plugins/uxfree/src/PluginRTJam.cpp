#include "PluginRTJam.hpp"

#define MAX_FIFO_FRAME_SIZE 1024

PluginRTJam::PluginRTJam() {
  for (int i=0; i<NUM_OUTPUTS; i++) {
    m_outputs[i] = new float[MAX_FIFO_FRAME_SIZE];
  }
}

PluginRTJam::~PluginRTJam() {
  for (int i=0; i<NUM_OUTPUTS; i++) {
    delete m_outputs[i];
  }
}

void PluginRTJam::connect(const char* host, int port, uint32_t id) {
  m_jamMixer.reset();
  m_jamSocket.isActivated = true;
  m_jamSocket.initClient(host, port, id);

}

void PluginRTJam::run(const float** inputs, float** outputs, uint32_t frames) {
  m_jamMixer.addLocalMonitor(inputs, frames);
  m_jamSocket.sendPacket((const float**)inputs, frames);
  m_jamSocket.readPackets(&m_jamMixer);

  m_jamMixer.getMix(m_outputs, frames);
  for ( int i = 0; i < 2; i++ ) {
    memcpy(outputs[i], m_outputs[i], frames * sizeof(float));
  }
}