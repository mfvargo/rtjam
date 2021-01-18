#!/bin/bash
#/usr/bin/jackd -dalsa -r48000 -p64 -n2 -D -Chw:SigmaI2SCodec,1 -Phw:SigmaI2SCodec,0
wget -O /home/pi/rtjam music.basscleftech.com/pi/rtjam 
chmod +x /home/pi/rtjam
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
/usr/bin/jackd -dalsa -r48000 -p128 -n2 -D -Chw:SigmaI2SCodec,1 -Phw:SigmaI2SCodec,0 -i2 -o2 &
sleep 2
/home/pi/rtjam &
/home/pi/MVerb &
sleep 2
jack_connect RTJam:out1 system:playback_1
jack_connect RTJam:out2 system:playback_2
jack_connect system:capture_1 MVerb:in1
jack_connect MVerb:out1 RTJam:in1
jack_connect system:capture_2 RTJam:in2
sleep 1
