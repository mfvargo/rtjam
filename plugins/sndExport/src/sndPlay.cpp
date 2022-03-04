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
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pwd.h>

#include "JamNetStuff.hpp"

#define MAX_FIFO_FRAME_SIZE 1024
#define NUM_OUTPUTS MAX_JAMMERS * 2 + 2

jack_port_t **input_ports;
jack_port_t **output_ports;
jack_client_t *client;

JamNetStuff::ReplayStream replay;
float *m_outputs[NUM_OUTPUTS];
int framecount = 0;

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
  float *inputs[2];
  float *outputs[2];

  inputs[0] = (float *)jack_port_get_buffer(input_ports[0], nframes);
  inputs[1] = (float *)jack_port_get_buffer(input_ports[1], nframes);
  outputs[0] = (float *)jack_port_get_buffer(output_ports[0], nframes);
  outputs[1] = (float *)jack_port_get_buffer(output_ports[1], nframes);
  // read audio from replay stream
  JamNetStuff::JamPacket *pPlaybackPacket = replay.getPlayBackMix();
  if (pPlaybackPacket)
  {
    // we do stuff here to decode the mix and feed it.
    pPlaybackPacket->decodeJamBuffer(outputs);
  }
  else
  {
    memcpy(outputs[0], inputs[0], sizeof(float) * nframes);
    memcpy(outputs[1], inputs[1], sizeof(float) * nframes);
  }
  return 0;
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown(void *arg)
{
  free(input_ports);
  free(output_ports);
  exit(1);
}

int main(int argc, char *argv[])
{
  JamNetStuff::JamMixer *pMixer = new JamNetStuff::JamMixer();
  pMixer->reset();
  json config;
  int i;
  const char **ports;
  const char *client_name = "testHarness";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  // Allocate output buffers
  for (int i = 0; i < NUM_OUTPUTS; i++)
  {
    m_outputs[i] = new float[MAX_FIFO_FRAME_SIZE];
  }

  /* open a client connection to the JACK server */

  client = jack_client_open(client_name, options, &status, server_name);
  if (client == NULL)
  {
    fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed)
    {
      fprintf(stderr, "Unable to connect to JACK server\n");
    }
    exit(1);
  }
  if (status & JackServerStarted)
  {
    fprintf(stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique)
  {
    client_name = jack_get_client_name(client);
    fprintf(stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback(client, process, pMixer);

  jack_on_shutdown(client, jack_shutdown, 0);

  /* create two ports pairs*/
  input_ports = (jack_port_t **)calloc(2, sizeof(jack_port_t *));
  output_ports = (jack_port_t **)calloc(2, sizeof(jack_port_t *));

  char port_name[16];
  for (i = 0; i < 2; i++)
  {
    sprintf(port_name, "input_%d", i + 1);
    input_ports[i] = jack_port_register(client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    sprintf(port_name, "output_%d", i + 1);
    output_ports[i] = jack_port_register(client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    if ((input_ports[i] == NULL) || (output_ports[i] == NULL))
    {
      fprintf(stderr, "no more JACK ports available\n");
      exit(1);
    }
  }

  /* Tell the JACK server that we are ready to roll.  Our
   * process() callback will start running now. */

  if (jack_activate(client))
  {
    fprintf(stderr, "cannot activate client");
    exit(1);
  }

  /* Connect the ports.  You can't do this before the client is
   * activated, because we can't make connections to clients
   * that aren't running.  Note the confusing (but necessary)
   * orientation of the driver backend ports: playback ports are
   * "input" to the backend, and capture ports are "output" from
   * it.
   */

  ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical | JackPortIsOutput);
  if (ports == NULL)
  {
    fprintf(stderr, "no physical capture ports\n");
    exit(1);
  }

  for (i = 0; i < 2; i++)
    if (jack_connect(client, ports[i], jack_port_name(input_ports[i])))
      fprintf(stderr, "cannot connect input ports\n");

  free(ports);

  ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical | JackPortIsInput);
  if (ports == NULL)
  {
    fprintf(stderr, "no physical playback ports\n");
    exit(1);
  }

  for (i = 0; i < 2; i++)
    if (jack_connect(client, jack_port_name(output_ports[i]), ports[i]))
      fprintf(stderr, "cannot connect input ports\n");

  free(ports);

  /* install a signal handler to properly quits jack client */
  signal(SIGQUIT, signal_handler);
  signal(SIGTERM, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGINT, signal_handler);

  // let's try to change thre frame size
  if (jack_set_buffer_size(client, 128))
    fprintf(stderr, "cannot set buffer size\n");

  /* keep running until the transport stops */
  replay.m_debug = true;
  while (1)
  {
    std::string input_line;
    std::getline(std::cin, input_line);
    string status = replay.readOpen(argv[1]);
    if (status == "playing")
    {
      json metadata = json::parse(replay.getMetdata());
      cout << metadata.dump() << endl;
      cout << "Opened up " << endl;
    }
    else
    {
      cout << "Failed to open: " << status << endl;
    }
  }
  jack_client_close(client);
  exit(0);
}
