#include "PluginRTJam.hpp"

#define MAX_FIFO_FRAME_SIZE 1024

PluginRTJam::PluginRTJam() {
  m_framecount = 0;
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
  m_framecount += frames;
  m_jamMixer.addLocalMonitor(inputs, frames);
  m_jamSocket.sendPacket((const float**)inputs, frames);
  m_jamSocket.readPackets(&m_jamMixer);
  m_jamMixer.getMix(m_outputs, frames);

  uint32_t ids[MAX_JAMMERS];
  m_jamSocket.getClientIds(ids);

  // Get input levels
  float leftPow = 0.0;
  float rightPow = 0.0;
  for (uint32_t i=0; i<frames; i++) {
      leftPow += pow(inputs[0][i], 2);
      rightPow += pow(inputs[1][i], 2);
  }
  leftPow /= frames + 1;
  if (leftPow > 1E-6) {
      leftPow = 10 * log10(leftPow);
  } else {
      leftPow = -60.0f;
  }
  rightPow /= frames + 1;
  if (rightPow > 1E-6) {
      rightPow = 10 * log10(rightPow);
  } else {
      rightPow = -60.0f;
  }
  leftInput.addSample(leftPow);
  rightInput.addSample(rightPow);

  // Store organized levels
  for (int i = 0; i < MIX_CHANNELS; i++) {
    m_levels.channelLevels[i] = m_jamMixer.channelLevels[i];
    m_levels.bufferDepths[i] = m_jamMixer.bufferDepths[i];
  }
  for (int i=0; i< MAX_JAMMERS; i++) {
    m_levels.clientIds[i] = ids[i];
  }
  m_levels.masterLevel = m_jamMixer.masterLevel;
  m_levels.inputLeft = leftInput.mean;
  m_levels.inputRight = rightInput.mean;
  m_levels.beat = m_jamMixer.getBeat();

  // Update levels in shared mem
  if (m_framecount > 24000) {
    // every 500 msec
    m_framecount = 0;
    memcpy(m_levelData.m_pJamLevels, &m_levels, sizeof(RTJamLevels));
    m_levelData.unlock();
  }


  for ( int i = 0; i < 2; i++ ) {
    memcpy(outputs[i], m_outputs[i], frames * sizeof(float));
  }
}