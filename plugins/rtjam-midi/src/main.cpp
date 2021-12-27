#include <alsa/asoundlib.h>
#include <string>
#include <array>
#include <unistd.h>
#include <memory>
#include "LevelData.hpp"
#include "MidiEvent.hpp"

using namespace std;

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
  CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0),
      "Could not open sequencer");

  CHK(snd_seq_set_client_name(seq_handle, "rtjam-midi"),
      "Could not set client name");
  CHK(in_port = snd_seq_create_simple_port(seq_handle, "listen:in",
                                           SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                           SND_SEQ_PORT_TYPE_APPLICATION),
      "Could not open port");
}

snd_seq_event_t *midi_read(void)
{
  snd_seq_event_t *ev = NULL;
  CHK(snd_seq_event_input(seq_handle, &ev), "Error reading Midi Input");
  return ev;
}

LevelData levelData;

void midi_process(const snd_seq_event_t *ev)
{
  unsigned char *pBuf = &levelData.m_pRingBuffer->ringBuffer[levelData.m_pRingBuffer->writeIdx++ * 28];
  memcpy(pBuf, ev, sizeof(snd_seq_event_t));
  levelData.m_pRingBuffer->writeIdx % 32;
  MidiEvent mEvent(ev);
  mEvent.dump();

  // if ((ev->type == SND_SEQ_EVENT_NOTEON) || (ev->type == SND_SEQ_EVENT_NOTEOFF))
  // {
  //   const char *type = (ev->type == SND_SEQ_EVENT_NOTEON) ? "on " : "off";
  //   printf("[%d] Note %s: %2x vel(%2x)\n", ev->time.tick, type,
  //          ev->data.note.note,
  //          ev->data.note.velocity);
  // }
  // else if (ev->type == SND_SEQ_EVENT_CONTROLLER)
  //   printf("[%d] Control:  %2x val(%2x)\n", ev->time.tick,
  //          ev->data.control.param,
  //          ev->data.control.value);
  // else
  //   printf("[%d] Unknown:  Unhandled Event Received\n", ev->time.tick);
}

int main()
{
  midi_open();
  // This should connect the pedal to the port here.
  execMyCommand("aconnect 32 129");

  while (1)
    midi_process(midi_read());
  return -1;
}
