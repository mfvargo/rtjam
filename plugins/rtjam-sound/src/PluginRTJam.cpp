#include "PluginRTJam.hpp"
#include <string.h>
#include <math.h>

#define MAX_FIFO_FRAME_SIZE 1024

static const float kAMP_DB = 8.656170245f;

void paramFetch(PluginRTJam *pJamPlugin)
{
  pJamPlugin->paramFlush();
  while (1)
  {
    pJamPlugin->getParams();
  }
}

PluginRTJam::PluginRTJam()
{
  m_framecount = 0;
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    m_outputs[i] = new float[MAX_FIFO_FRAME_SIZE];
  }
}

PluginRTJam::~PluginRTJam()
{
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    delete m_outputs[i];
  }
}

void PluginRTJam::init()
{
  for (int i = 0; i < 2; i++)
  {
    m_pedalBoards[i].init();
  }
  // write the effect chain json data
  syncConfigData();
  m_threads.push_back(std::thread(paramFetch, this));
}

void PluginRTJam::syncLevels()
{
  memcpy(m_levelData.m_pJamLevels, &m_levels, sizeof(RTJamLevels));
  // m_levelData.unlock();
}

void PluginRTJam::syncConfigData()
{
  json config = json::array();
  for (int i = 0; i < 2; i++)
  {
    char name[64];
    sprintf(name, "channel_%d", i);
    config.push_back(m_pedalBoards[i].m_effectChain.getChainConfig(name, i));
  }
  sprintf(m_levelData.m_pJsonInfo, "%s", config.dump().c_str());
}

void PluginRTJam::paramFlush()
{
  m_paramData.flush();
}

void PluginRTJam::getParams()
{
  m_paramData.receive(&m_param);
  printf("received param %d: %s, %f, %d, %d\n", m_param.param, m_param.sValue, m_param.fValue, m_param.iValue, m_param.iValue2);
  switch (m_param.param)
  {
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
    if (m_param.fValue < -59.9)
    {
      m_param.fValue = -60.0;
    }
    m_jamMixer.gains[m_param.param - paramChanGain1] = SignalBlock::dbToFloat(m_param.fValue);
    break;
  case paramMasterVol:
    m_jamMixer.masterVol = SignalBlock::dbToFloat(m_param.fValue);
    break;
  case paramReverbMix:
    // Legacy param when we just had one reverb
    break;
  case paramRoomChange:
    connect(m_param.sValue, m_param.iValue, m_param.iValue2);
    break;
  case paramDisconnect:
    disconnect();
    break;
  case paramHPFOn:
    // deprecated.  This is done with paramSetEffectConfig instead
    break;
  case paramHPFOff:
    // deprecated.  This is done with paramSetEffectConfig instead
    break;
  case paramReverbOne:
    // deprecated.  This is done with paramSetEffectConfig instead
    break;
  case paramReverbTwo:
    // deprecated.  This is done with paramSetEffectConfig instead
    break;
  case paramGetConfigJson:
    syncConfigData();
    break;
  case paramSetEffectConfig:
    if ((m_param.iValue >= 0 && m_param.iValue < 2) &&
        (m_param.iValue2 >= 0 && m_param.iValue2 < m_pedalBoards[m_param.iValue].m_effectChain.size()))
    {
      try
      {
        Effect *pEffect = m_pedalBoards[m_param.iValue].m_effectChain.getEffect(m_param.iValue2);
        cerr << json::parse(m_param.sValue).dump(2) << endl;
        pEffect->setSettingValue(json::parse(m_param.sValue));
      }
      catch (json::exception &e)
      {
        cerr << e.what() << endl;
      }
      catch (...)
      {
        cerr << "failed to parse json!" << endl;
      }
    }
  }
}

void PluginRTJam::connect(const char *host, int port, uint32_t id)
{
  // m_jamMixer.reset();
  m_jamSocket.isActivated = true;
  m_jamSocket.initClient(host, port, id);
  // Need to flush out any residual data in the socket from end of last session
  m_jamSocket.readPackets(NULL);
}

void PluginRTJam::disconnect()
{
  m_jamSocket.isActivated = false;
}

void PluginRTJam::run(const float **inputs, float **outputs, uint32_t frames)
{
  m_framecount += frames;

  // Setup itermediate buffer for processed inputs
  float *tempOut[2];
  float oneBuffOut[frames];
  float twoBuffOut[frames];
  tempOut[0] = oneBuffOut;
  tempOut[1] = twoBuffOut;

  // run the effect chains
  m_pedalBoards[0].m_effectChain.process(inputs[0], oneBuffOut, frames);
  m_pedalBoards[1].m_effectChain.process(inputs[1], twoBuffOut, frames);

  // Add to local monitor
  m_jamMixer.addLocalMonitor((const float **)tempOut, frames);

  // Send to the room
  m_jamSocket.sendPacket((const float **)tempOut, frames);

  // read any data from the network
  m_jamSocket.readPackets(&m_jamMixer);

  // Get the mix
  m_jamMixer.getMix(m_outputs, frames);

  uint32_t ids[MAX_JAMMERS];
  m_jamSocket.getClientIds(ids);

  // Get input levels  (from tempOut which is what we sent to the room.)
  float leftPow = 0.0;
  float rightPow = 0.0;
  for (uint32_t i = 0; i < frames; i++)
  {
    leftPow += pow(tempOut[0][i], 2);
    rightPow += pow(tempOut[1][i], 2);
  }
  leftPow /= frames + 1;
  if (leftPow > 1E-6)
  {
    leftPow = 10 * log10(leftPow);
  }
  else
  {
    leftPow = -60.0f;
  }
  rightPow /= frames + 1;
  if (rightPow > 1E-6)
  {
    rightPow = 10 * log10(rightPow);
  }
  else
  {
    rightPow = -60.0f;
  }
  leftInput.addSample(leftPow);
  rightInput.addSample(rightPow);

  // Communicate light values
  m_lightData.m_pLightSettings->inputOne = dbToColor(leftInput.mean);
  m_lightData.m_pLightSettings->inputTwo = dbToColor(rightInput.mean);

  // Store organized levels
  for (int i = 0; i < MIX_CHANNELS; i++)
  {
    m_levels.channelLevels[i] = m_jamMixer.channelLevels[i];
    m_levels.bufferDepths[i] = m_jamMixer.bufferDepths[i];
    m_levels.peakLevels[i] = m_jamMixer.peakLevels[i];
  }
  for (int i = 0; i < MAX_JAMMERS; i++)
  {
    m_levels.clientIds[i] = ids[i];
  }
  m_levels.masterLevel = m_jamMixer.masterLevel;
  m_levels.peakMaster = m_jamMixer.masterPeak;
  m_levels.inputLeft = leftInput.mean;
  m_levels.inputRight = rightInput.mean;
  m_levels.peakLeft = leftInput.peak;
  m_levels.peakRight = rightInput.peak;
  m_levels.beat = m_jamMixer.getBeat();
  m_levels.isConnected = m_jamSocket.isActivated;

  this->syncLevels();

  for (int i = 0; i < 2; i++)
  {
    memcpy(outputs[i], m_outputs[i], frames * sizeof(float));
  }
}

LightColors PluginRTJam::dbToColor(float power)
{
  if (power < -45.0)
  {
    return (black);
  }
  else if (power < -30)
  {
    return (green);
  }
  else if (power < -20)
  {
    return (orange);
  }
  return red;
}