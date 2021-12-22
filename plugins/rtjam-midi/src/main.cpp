#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <jack/jack.h>
#include <jack/midiport.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <pwd.h>
#include "LevelData.hpp"

jack_port_t *input_port;
jack_client_t *client = NULL;
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

// Called on signals to close jack connection neatly
static void signal_handler(int sig)
{
  jack_client_close(client);
  fprintf(stderr, "signal received, exiting ...\n");
  exit(0);
}

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 */

int process(jack_nframes_t nframes, void *arg)
{
  void *inport_buf = jack_port_get_buffer(input_port, nframes);
  jack_midi_event_t in_event;
  jack_nframes_t event_count = jack_midi_get_event_count(inport_buf);
  if (event_count != 0)
  {
    jack_midi_event_t in_event;
    for (jack_nframes_t i = 0; i < event_count; i++)
    {
      jack_midi_event_get(&in_event, inport_buf, i);
      unsigned char *pBuf = &levelData.m_pRingBuffer->ringBuffer[levelData.m_pRingBuffer->writeIdx++ * 3];
      memcpy(pBuf, in_event.buffer, 3);
      levelData.m_pRingBuffer->writeIdx % 32;
    }
  }
  return 0;
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown(void *arg)
{
  free(input_port);
  exit(1);
}

int main(int argc, char *argv[])
{
  int i;
  const char **ports;
  const char *client_name = "rtjam-midi";
  const char *server_name = NULL;
  jack_options_t options = JackNoStartServer;
  jack_status_t status;

  // loop while trying to connect to jack.  If jack is not running this will just keep looping
  // until it starts.
  while (client == NULL)
  {
    /* open a client connection to the JACK server */
    client = jack_client_open(client_name, options, &status);
    if (client == NULL)
    {
      fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
      sleep(3);
    }
  }

  // If we got here, jack is running.  Let's try to start a2j midi bridge
  execMyCommand("a2j_control start");
  execMyCommand("aconnect 20 14");

  if (status & JackNameNotUnique)
  {
    client_name = jack_get_client_name(client);
    fprintf(stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback(client, process, NULL);

  jack_on_shutdown(client, jack_shutdown, 0);

  input_port = jack_port_register(client, "in", JACK_DEFAULT_MIDI_TYPE, (JackPortIsInput | JackPortIsTerminal), 0);

  /* Tell the JACK server that we are ready to roll.  Our
     * process() callback will start running now. */

  if (jack_activate(client))
  {
    fprintf(stderr, "cannot activate client");
    exit(1);
  }

  // Lets try to connect the midi-port to something
  ports = jack_get_ports(client, "a2j", NULL, JackPortIsOutput);
  if (ports == NULL)
  {
    fprintf(stderr, "no a2j ports running.  check a2j_control --status \n");
    exit(1);
  }
  cout << ports[0] << endl;
  if (jack_connect(client, ports[0], jack_port_name(input_port)))
    fprintf(stderr, "cannot connect to a2j port\n");

  jack_free(ports);

  /* install a signal handler to properly quits jack client */
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);

  /* keep running until the transport stops */
  while (1)
  {
    sleep(1);
  }
  jack_client_close(client);
  exit(0);
}