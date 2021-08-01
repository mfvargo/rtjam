#include "PluginRTJam.hpp"
#include "Settings.hpp"
#include <iostream>
#include <fstream>

#include <string.h>
#include <math.h>

#define MAX_FIFO_FRAME_SIZE 1024

static const float kAMP_DB = 8.656170245f;

bool isRunning = true;

void paramFetch(PluginRTJam *pJamPlugin)
{
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
    m_pedalBoards[i].init(json::array());
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
    config.push_back(m_pedalBoards[i].getChainConfig(name, i));
  }
  sprintf(m_levelData.m_pJsonInfo, "%s", config.dump().c_str());
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
    if ((m_param.iValue >= 0 && m_param.iValue < 2) && (m_param.iValue2 >= 0))
    {
      try
      {
        m_pedalBoards[m_param.iValue].setEffectSetting(json::parse(m_param.sValue), m_param.iValue2);
        cerr << json::parse(m_param.sValue).dump(2) << endl;
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
  case paramInsertPedal:
    if ((m_param.iValue >= 0 && m_param.iValue < 2) && (m_param.iValue2 >= 0))
    {
      m_pedalBoards[m_param.iValue].insertPedal(m_param.iValue2, m_param.sValue);
      syncConfigData();
    }
    break;
  case paramDeletePedal:
    if ((m_param.iValue >= 0 && m_param.iValue < 2) && (m_param.iValue2 >= 0))
    {
      m_pedalBoards[m_param.iValue].deletePedal(m_param.iValue2);
      syncConfigData();
    }
    break;
  case paramMovePedal:
    if ((m_param.iValue >= 0 && m_param.iValue < 2) && (m_param.iValue2 >= 0))
    {
      m_pedalBoards[m_param.iValue].movePedal(m_param.iValue2, m_param.fValue);
      syncConfigData();
    }
    break;
  case paramLoadBoard:
    try
    {
      if (m_param.iValue >= 0 && m_param.iValue < 2)
      {
        m_pedalBoards[m_param.iValue].init(json::parse(m_param.sValue)["config"]);
        syncConfigData();
      }
    }
    catch (...)
    {
      cerr << "failed to parse json!" << endl;
    }
    break;
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

  // Save the input power levels
  // Get input levels  (from tempOut which is what we sent to the room.)
  m_leftInput.addSample(SignalBlock::getFramePower(inputs[0], frames));
  m_rightInput.addSample(SignalBlock::getFramePower(inputs[1], frames));

  // run the effect chains
  m_pedalBoards[0].process(inputs[0], oneBuffOut, frames);
  m_pedalBoards[1].process(inputs[1], twoBuffOut, frames);

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

  // Get the levels post effects  (from tempOut which is what we sent to the room.)
  m_leftRoomInput.addSample(SignalBlock::getFramePower(tempOut[0], frames));
  m_rightRoomInput.addSample(SignalBlock::getFramePower(tempOut[1], frames));

  // Communicate light values
  m_lightData.m_pLightSettings->inputOne = dbToColor(m_leftInput.mean);
  m_lightData.m_pLightSettings->inputTwo = dbToColor(m_rightInput.mean);

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
  m_levels.inputLeft = m_leftInput.mean;
  m_levels.inputRight = m_rightInput.mean;
  m_levels.peakLeft = m_leftInput.peak;
  m_levels.peakRight = m_rightInput.peak;
  m_levels.roomLevelLeft = m_leftRoomInput.mean;
  m_levels.roomPeakLeft = m_leftRoomInput.peak;
  m_levels.roomLevelRight = m_rightRoomInput.mean;
  m_levels.roomPeakRight = m_rightRoomInput.peak;
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