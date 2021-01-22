#!/bin/bash
#/usr/bin/jackd -dalsa -r48000 -p64 -n2 -D -Chw:SigmaI2SCodec,1 -Phw:SigmaI2SCodec,0
wget --tries=1 -T 2 -O /home/pi/rtjam music.basscleftech.com/pi/rtjam 
chmod +x /home/pi/rtjam
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
jack_control start
jack_control ds alsa
jack_control dps device hw:USB
jack_control dps rate 48000
jack_control dps nperiods 2
jack_control dps period 128
sleep 2
/home/pi/rtjam &
sleep 2
jack_connect RTJam:out1 system:playback_1
jack_connect RTJam:out2 system:playback_2
jack_connect system:capture_1 RTJam:in1
jack_connect system:capture_2 RTJam:in2
sleep 1
