#include "PluginRTJam.hpp"
#include <string.h>
#include <math.h>

#define MAX_FIFO_FRAME_SIZE 1024

static const float kAMP_DB = 8.656170245f;


void levelPush(PluginRTJam* pJamPlugin) {
  while (1) {
    std::this_thread::sleep_for(std::chrono::microseconds(1000000));
    pJamPlugin->syncLevels();
  }
}

void paramFetch(PluginRTJam* pJamPlugin) {
  pJamPlugin->paramFlush();
  while(1) {
    pJamPlugin->getParams();
  }
}

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

void PluginRTJam::init() {
  printf("mac: %s", m_jamSocket.getMacAddress().c_str());
  m_threads.push_back(std::thread(levelPush, this)); 
  m_threads.push_back(std::thread(paramFetch, this)); 
}

void PluginRTJam::syncLevels() {
  memcpy(m_levelData.m_pJamLevels, &m_levels, sizeof(RTJamLevels));
  m_levelData.unlock();
}

void PluginRTJam::paramFlush() {
  m_paramData.flush();
}

float PluginRTJam::dbToFloat(float value) {
    if (value < -59.5) {
        return 0.0f;
    }
    return std::exp( (value/72.0f) * 72.0f / kAMP_DB);
}

void PluginRTJam::getParams() {
  m_paramData.receive(&m_param);
  printf("received param %d: %s, %f, %d, %d\n", m_param.param, m_param.sValue, m_param.fValue, m_param.iValue, m_param.iValue2);
  switch(m_param.param) {
    case paramChanGain1:
    case paramChanGain2:
    case paramChanGain3:
    case paramChanGain4:
    case paramChanGain5:
    case paramChanGain6:
    case paramChanGain7:
    case paramChanGain8:
    case paramChanGain9:
    case paramChanGain10:
    case paramChanGain11:
    case paramChanGain12:
    case paramChanGain13:
    case paramChanGain14:
      if (m_param.fValue < -29.9) {
          m_param.fValue = -60.0;
      }
      m_jamMixer.gains[m_param.param - paramChanGain1] = dbToFloat(m_param.fValue);
      break;
    case paramMasterVol:
      m_jamMixer.masterVol = dbToFloat(m_param.fValue);
      break;
    case paramReverbMix:
      // fVerb.setParameter(MVerb<float>::MIX, value);
      break;
    case paramRoomChange:
      connect(m_param.sValue, m_param.iValue, m_param.iValue2);
      break;
    case paramDisconnect:
      disconnect();
      break;
  }
}

void PluginRTJam::connect(const char* host, int port, uint32_t id) {
  m_jamMixer.reset();
  m_jamSocket.isActivated = true;
  m_jamSocket.initClient(host, port, id);
}

void PluginRTJam::disconnect() {
  m_jamMixer.reset();
  m_jamSocket.isActivated = false;
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
  m_levels.isConnected = m_jamSocket.isActivated;

  for ( int i = 0; i < 2; i++ ) {
    memcpy(outputs[i], m_outputs[i], frames * sizeof(float));
  }
}