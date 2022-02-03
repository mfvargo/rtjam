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
#include "JamEngine.hpp"
#include "Settings.hpp"

jack_port_t **input_ports;
jack_port_t **output_ports;
jack_client_t *client = NULL;

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
    JamEngine *pJamEngine = (JamEngine *)arg;
    float tbufIn[nframes];
    float *inputs[2];
    float *outputs[2];
    inputs[0] = (float *)jack_port_get_buffer(input_ports[0], nframes);
    if (input_ports[1])
    {
        inputs[1] = (float *)jack_port_get_buffer(input_ports[1], nframes);
    }
    else
    {
        memset(tbufIn, 0x00, sizeof(float) * nframes);
        inputs[1] = tbufIn;
    }
    outputs[0] = (float *)jack_port_get_buffer(output_ports[0], nframes);
    outputs[1] = (float *)jack_port_get_buffer(output_ports[1], nframes);
    // Call the run function
    pJamEngine->run((const float **)inputs, outputs, nframes);
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

    int i;
    const char **ports;
    const char *client_name = "rtjam";
    jack_options_t options = JackNoStartServer;
    jack_status_t status;

    JamEngine *pJamEngine = new JamEngine();
    pJamEngine->init();
    // Auto connect for Kevin Kirkpatrick
    Settings settings;
    settings.loadFromFile();
    std::string serverName = settings.getOrSetValue("server", std::string(SERVER_NAME));
    int port = settings.getOrSetValue("port", 7891);
    int clientId = rand() % 32768;
    clientId = settings.getOrSetValue("clientId", clientId);
    if (settings.getOrSetValue("autoconnect", 0) != 0)
    {
        pJamEngine->connect(serverName.c_str(), port, clientId);
    }
    settings.saveToFile();

    // loop while trying to connect to jack.  If jack is not running this will just keep looping
    // until it starts.
    while (client == NULL)
    {
        /* open a client connection to the JACK server */
        client = jack_client_open(client_name, options, &status);
        if (client == NULL)
        {
            fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
            sleep(1);
        }
    }

    if (status & JackNameNotUnique)
    {
        client_name = jack_get_client_name(client);
        fprintf(stderr, "unique name `%s' assigned\n", client_name);
    }

    jack_set_process_callback(client, process, pJamEngine);

    jack_on_shutdown(client, jack_shutdown, 0);

    /* create two ports pairs*/
    input_ports = (jack_port_t **)calloc(2, sizeof(jack_port_t *));
    output_ports = (jack_port_t **)calloc(2, sizeof(jack_port_t *));

    char port_name[16];
    int inportCount = 0;
    for (i = 0; i < 2; i++)
    {
        sprintf(port_name, "input_%d", i + 1);
        input_ports[i] = jack_port_register(client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        if (input_ports[i] != NULL)
        {
            inportCount++;
        }
        else
        {
            fprintf(stderr, "input port %d unavailable\n", i); // don't exit, we can run with just one.
        }
    }

    for (i = 0; i < 2; i++)
    {
        sprintf(port_name, "output_%d", i + 1);
        output_ports[i] = jack_port_register(client, port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if (output_ports[i] == NULL)
        {
            fprintf(stderr, "no more JACK output ports available\n");
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

    for (i = 0; i < inportCount; i++)
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
    while (1)
    {
        sleep(1);
    }
    jack_client_close(client);
    exit(0);
}
