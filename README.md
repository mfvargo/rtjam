# RTJam

Real Time Music Jam

The Real Time Music Jam software is intended to provide low latency audio over the network that will enable performers to make music as if they were in the same place. The system is comprised of a broadcast server that listens on a UDP port for jam packets and some number of clients that create/consume audio data.

The server keeps a table of up to 7 jammers in the "room" and will forward each packet it receives to the other jammers in the room.  It does not transmit the packet back to the orginator (client provides local monitoring).  The server starts 4 separate threads of excecution each one hosting a room on UDP ports 7891-7894.

Each jam packet has two separate channels of audio so the jammers can have a channel for voice and guitar, stereo keyboards, or whatever they choose.  The two channels are isochronous for that person.

So in this way a room consists of up to 7 jammers each with two channels or 14 separate audio channels

## Latency Breakdown


* Audio system running 64 sample frames with 2 buffers - Jack delay = 2.67msec
* Jitter Buffer default 512 samples = 8 frames of data at at 1.33msec / frame = 10.6msec
* Network latency (speed of light and buffers in the network) = variable depending on locations and networks.  Figure speed of light is 200km/msec (through glass).


Measuring on my local network I can get best case of 13msec.  Thats with an avg jitter buffer depth of 365 samples (7.6msec), 2.67msec jack delay, x amount of stack delay.  I'm measuring this at the analog interface of my USB 2.0 audio device.  The device has a local mix so I can go 50/50 and see the delay from a mic tap.  Hard to hear, but it's there.

So the bulk of the delay is network transit and jitter buffer.  


## Get The code
```
git clone --recurse-submodules https://github.com/mfvargo/rtjam.git
cd rtjam
make
```

## Components

### VST Plugin (bin/rtjam.vst.so)
The RTJam plugin is a VST built using the DISTRO/DPF open source framework https://github.com/DISTRHO/DPF 

The Plugin will take frames from it's input and push them using UDP to a jamrtime broadcast server.  It will then receive packets from the server that originate from other jamrtime users.  The VST then mixes those channels into a stereo output.  That's the room.

The VST has 14 outputs.  The first two are the mixed channels based on the sliders in the VST.  Outputs 3-12 are the individual channels from the participants in the room.  Using the VST in a DAW package allows all 8 channels to be recorded separately for later mixing.

### Jack Standalone (bin/rtjam)
The RTJam software also builds on the Raspberry Pi 4 and can be run as a "standalone JACK" application.  This has been the most successful implementation for playing music realtime on the internet.  The Pi has a very stable multimedia jack port that can run 64 sample frames with only 2 period of buffer without the dreaded XRUN issues you see on most other platforms.

### App U/X

![Pi App](Standalone.png)

- Room selector - chooses which jam room to occupy
- Master volume - slider for overall level in the outputs
- Bottom left faders - Shows pre and post fader levels for the users inputs
- Player controls (right half side) - Each control set corresponds to another person in the room. The controls are level sliders for that user's two channels.

### Broadcast Server
The server just listens for packets from VST clients.  The server dynamically creates channels based on the source address of the client packets and forwards packets to all active listeners.  There is currently no session control.  When you start transmitting the server will allocate a channel to you if one is open.  If you don't send any packets for 1 second, your channel is made available.

The server also creates a room mix for each room it hosts.  The PCM output of this mix is written to a FIFO file that can be read by ices or some other podcasting software.
### TODOS
* make server name configurable
* ensure all clients running the same framerate
* allow clients with different sample rates to intermix
* add stereo panning to the channels to help people separate sources.
* make a listen only mode for a VST for live remixing of a room

### Hooking up a Raspberry Pi

https://youtu.be/iUXzOq8HfEQ

### Making the thing autostart on a Raspberry Pi

Add startup app to autostart folder
```
cd ~/.config
mkdir autostart
cd autostart
vi app.desktop
```
Contents of app.desktop:
```
[Desktop Entry]
Encoding=UTF-8
Typr=Application
Name=rtjam
Exec=/bin/bash /home/pi/Desktop/JamOn.sh
StartupNotify=false
Terminal=false
Hidden=-false
```
Contents of /home/pi/Desktop/JamOn.sh

```
#!/bin/bash
wget --tries=1 -T 2 -O /home/pi/rtjam music.basscleftech.com/pi/rtjam 
chmod +x /home/pi/rtjam
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
jack_control start
jack_control ds alsa
jack_control dps device hw:USB   # Note device name depends on USB device!
jack_control dps rate 48000
jack_control dps nperiods 2
jack_control dps period 128
sleep 2
cd
/home/pi/rtjam &
sleep 2
jack_connect RTJam:out1 system:playback_1
jack_connect RTJam:out2 system:playback_2
jack_connect system:capture_1 RTJam:in1
jack_connect system:capture_2 RTJam:in2
sleep 1

```
### Using With Sample DAW (Ardour)

You can use the VST in a DAW to record sessions for mixing.  In the screen shots below, I have two inputs routed into a channel with the RTJam plugin inserted.  I then route outputs 3-16 of the plugin to separate mono channels that can be record armed and tracked.  Post jam session those tracks can be mixed/engineered. It would also be possible to run a real time mix of those to provide a "house" mix for some kind of live show.

Mixer view of session

![Ardour Mixer](ArdourMixer.png)

Routing Grid (route the individual tracks from the VST to separate tracks)

![Routing Grid](RoutingGrid.png)
