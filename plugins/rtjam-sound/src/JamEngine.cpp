#include "JamEngine.hpp"
#include <iostream>
#include <fstream>

#include <string.h>
#include <math.h>

#define MAX_FIFO_FRAME_SIZE 1024

static const float kAMP_DB = 8.656170245f;

bool isRunning = true;

void paramFetch(JamEngine *pJamPlugin)
{
  while (1)
  {
    pJamPlugin->getParams();
  }
}

JamEngine::JamEngine()
{
  m_framecount = 0;
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    m_outputs[i] = new float[MAX_FIFO_FRAME_SIZE];
  }
  m_inputDCremoval[0].init(BiQuadFilter::FilterType::HighPass, 2.0, 1.0, 1.0, 48000);
  m_inputDCremoval[1].init(BiQuadFilter::FilterType::HighPass, 2.0, 1.0, 1.0, 48000);
  m_LeftRoomMute = false;
  m_RightRoomMute = false;
}

JamEngine::~JamEngine()
{
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    delete m_outputs[i];
  }
}

void JamEngine::init()
{
  for (int i = 0; i < 2; i++)
  {
    m_pedalBoards[i].init(json::array());
  }
  // write the effect chain json data
  syncConfigData();
  m_threads.push_back(std::thread(paramFetch, this));
}

void JamEngine::syncLevels()
{
  memcpy(m_levelData.m_pJamLevels, &m_levels, sizeof(RTJamLevels));
  // m_levelData.unlock();
}

void JamEngine::syncConfigData()
{
  json config = json::array();
  for (int i = 0; i < 2; i++)
  {
    char name[64];
    sprintf(name, "channel_%d", i);
    config.push_back(m_pedalBoards[i].getChainConfig(name, i));
  }
  sprintf(m_levelData.m_pJsonInfo, "%s", config.dump().c_str());
  // mark the time the json was updated
  m_levels.jsonTimeStamp = JamNetStuff::getMicroTime();
}

void JamEngine::getParams()
{
  m_paramData.receive(&m_param);

  cout << "received: " << m_param.param << endl;

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
    m_jamMixer.gains[m_param.param - paramChanGain1] = SignalBlock::dbToLinear(m_param.fValue);
    break;
  case paramMasterVol:
    m_jamMixer.masterVol = SignalBlock::dbToLinear(m_param.fValue);
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
    break;
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
        json boardInfo = json::parse(m_param.sValue);
        m_pedalBoards[m_param.iValue].init(boardInfo["config"]);
        m_pedalBoards[m_param.iValue].m_loadedBoardId = boardInfo["id"];
        syncConfigData();
      }
    }
    catch (...)
    {
      cerr << "failed to parse json!" << endl;
    }
    break;
  case paramTuneChannel:
    // Mute the channel
    if (m_param.iValue >= 0 && m_param.iValue < 2)
    {
      m_pedalBoards[m_param.iValue].tuner(m_param.iValue2 == 1);
    }
    break;
  case paramMetronomeVolume:
    // set the metronome sound type and volume
    m_jamMixer.setMetronomeConfig(m_param.iValue, m_param.fValue);
    break;
  case paramSetFader:
    if (m_param.iValue >= 0 && m_param.iValue < MIX_CHANNELS)
    {
      if (m_param.fValue >= -1.0 && m_param.fValue <= 1.0)
      {
        m_jamMixer.fades[m_param.iValue] = m_param.fValue;
      }
    }
    break;
  case paramMuteToRoom:
    if (m_param.iValue == 0)
    {
      m_LeftRoomMute = m_param.iValue2 == 1;
    }
    if (m_param.iValue == 1)
    {
      m_RightRoomMute = m_param.iValue2 == 1;
    }
    break;
  case paramConnectionKeepAlive:
    m_conTimer.reset();
    break;
  }
}

void JamEngine::connect(const char *host, int port, uint32_t id)
{
  // Log the time the connection starts
  m_conTimer.reset();
  // Turn on the socket
  m_jamSocket.isActivated = true;
  // Reset all the other player volumes in the mixer and flush all the jitterBuffers
  m_jamMixer.clearPlayerVolumes();
  // set the host endpoint
  m_jamSocket.initClient(host, port, id);
}

void JamEngine::disconnect()
{
  m_jamSocket.disconnect();
}

void JamEngine::run(const float **inputs, float **outputs, uint32_t frames)
{
  // Auto disconnect if we don't have a keepalive
  if (m_jamSocket.isActivated && m_conTimer.getTimeFromStart() > (uint64_t)10 * 60 * 1000 * 1000) // 10 minutes
  {
    // It's been 10 minutes since we connected or we got a paramConnectionKeepAlive
    // Time to disconnect
    // TODO: Put this back in once I figure why it's not working correctly.
    cout << "Disconnect timeout: " << m_conTimer.getTimeFromStart() << endl;
    m_conTimer.reset();
    disconnect();
  }

  m_framecount += frames;

  // Debug output
  if (m_framecount % 375 == 0)
  {
    // every second
    // cout << "Disconnect timeout: " << m_conTimer.getTimeFromStart() << endl;
    // m_jamMixer.dumpOut();
  }

  // Setup itermediate buffer for processed inputs
  float *tempOut[2];
  float oneBuffOut[frames];
  float twoBuffOut[frames];
  tempOut[0] = oneBuffOut;
  tempOut[1] = twoBuffOut;

  // Remove DC offset from samples
  m_inputDCremoval[0].getBlock(inputs[0], oneBuffOut, frames);
  m_inputDCremoval[1].getBlock(inputs[1], twoBuffOut, frames);

  // Save the input power levels
  // Get input levels  (from tempOut which is what we sent to the room.)
  m_leftInput.addSample(SignalBlock::getFramePower(oneBuffOut, frames));
  m_rightInput.addSample(SignalBlock::getFramePower(twoBuffOut, frames));

  // run the effect chains
  m_pedalBoards[0].process(oneBuffOut, oneBuffOut, frames);
  m_pedalBoards[1].process(twoBuffOut, twoBuffOut, frames);

  // Add to local monitor
  m_jamMixer.addLocalMonitor((const float **)tempOut, frames);

  // read any data from the network
  m_jamSocket.readPackets(&m_jamMixer);

  // Send to the room
  if (m_LeftRoomMute)
  {
    memset(tempOut[0], 0x00, frames * sizeof(float));
  }
  if (m_RightRoomMute)
  {
    memset(tempOut[1], 0x00, frames * sizeof(float));
  }
  m_jamSocket.sendPacket((const float **)tempOut, frames);

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
  m_levels.inputLeftFreq = m_pedalBoards[0].getFrequency();
  m_levels.inputRightFreq = m_pedalBoards[1].getFrequency();
  m_levels.leftTunerOn = m_pedalBoards[0].isTuning();
  m_levels.rightTunerOn = m_pedalBoards[1].isTuning();
  m_levels.leftRoomMute = m_LeftRoomMute;
  m_levels.rightRoomMute = m_RightRoomMute;
  syncLevels();

  for (int i = 0; i < 2; i++)
  {
    memcpy(outputs[i], m_outputs[i], frames * sizeof(float));
  }
}

void JamEngine::sendMidiEvent(unsigned char *buffer)
{
  unsigned char *pBuf = &m_levelData.m_pRingBuffer->ringBuffer[m_levelData.m_pRingBuffer->writeIdx++ * 3];
  memcpy(pBuf, buffer, 3);
  m_levelData.m_pRingBuffer->writeIdx % 32;
}

LightColors JamEngine::dbToColor(float power)
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