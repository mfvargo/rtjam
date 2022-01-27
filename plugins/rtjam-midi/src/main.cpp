#include <alsa/asoundlib.h>
#include <string>
#include <array>
#include <unistd.h>
#include <memory>
#include "LevelData.hpp"
#include "MidiEvent.hpp"

using namespace std;

LevelData levelData;

// Utility to shell out a command
string execMyCommand(string cmd)
{
  array<char, 128> buffer;
  string result;
  unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe)
  {
    return "popen() failed!";
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
  {
    result += buffer.data();
  }
  return result;
}

static snd_seq_t *seq_handle;
static int in_port;

#define CHK(stmt, msg)    \
  if ((stmt) < 0)         \
  {                       \
    puts("ERROR: " #msg); \
    exit(1);              \
  }
void midi_open(void)
{
  CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0),
      "Could not open sequencer");

  CHK(snd_seq_set_client_name(seq_handle, "rtjam-midi"),
      "Could not set client name");
  CHK(in_port = snd_seq_create_simple_port(seq_handle, "listen:in",
                                           SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                           SND_SEQ_PORT_TYPE_APPLICATION),
      "Could not open port");
}

int scanForPort()
{
  snd_seq_client_info_t *cinfo;
  snd_seq_port_info_t *pinfo;

  snd_seq_client_info_alloca(&cinfo);
  snd_seq_port_info_alloca(&pinfo);
  snd_seq_client_info_set_client(cinfo, -1);
  while (snd_seq_query_next_client(seq_handle, cinfo) >= 0)
  {
    /* reset query info */
    snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
    snd_seq_port_info_set_port(pinfo, -1);
    while (snd_seq_query_next_port(seq_handle, pinfo) >= 0)
    {
      int card = snd_seq_client_info_get_card(cinfo);
      if (card != -1)
      {
        cout << "Found " << snd_seq_client_info_get_name(cinfo) << endl;
        strcpy(levelData.m_pRingBuffer->deviceName, snd_seq_client_info_get_name(cinfo));
        return snd_seq_client_info_get_client(cinfo);
      }
    }
  }
  return -1;
}

snd_seq_event_t *midi_read(void)
{
  snd_seq_event_t *ev = NULL;
  CHK(snd_seq_event_input(seq_handle, &ev), "Error reading Midi Input");
  return ev;
}

void midi_process(const snd_seq_event_t *ev)
{
  unsigned char *pBuf = &levelData.m_pRingBuffer->ringBuffer[levelData.m_pRingBuffer->writeIdx++ * 28];
  memcpy(pBuf, ev, sizeof(snd_seq_event_t));
  levelData.m_pRingBuffer->writeIdx % 32;
  MidiEvent mEvent(ev);
  mEvent.dump();
}

int main()
{
  midi_open();

  int port = scanForPort();
  // This should connect the pedal to the port here.
  string cmd = "aconnect " + std::to_string(port) + " " + std::to_string(snd_seq_client_id(seq_handle));
  cout << cmd << endl;
  cout << execMyCommand(cmd) << endl;

  while (1)
    midi_process(midi_read());
  return -1;
}
