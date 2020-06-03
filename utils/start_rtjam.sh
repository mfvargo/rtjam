#!/bin/bash
jack_control start
jack_control ds alsa
jack_control dps device hw:USB
jack_control dps rate 48000
jack_control dps nperiods 2
jack_control dps period 64
sleep 2
/home/pi/rtjam &
sleep 3
jack_connect RTJam:out1 system:playback_1
jack_connect RTJam:out2 system:playback_2
jack_connect system:capture_1 RTJam:in1
jack_connect system:capture_2 RTJam:in2
sleep 1