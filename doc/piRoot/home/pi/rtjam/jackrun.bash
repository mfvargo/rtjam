#!/bin/bash
# Check for soundin.cfg
if [ -f soundin.cfg ];
then
  INDEV=`cat soundin.cfg`
else
  INDEV=hw:USB
  echo $INDEV > soundin.cfg
fi
# make sure there was something in the file
if [ -z ${INDEV} ];
then 
  INDEV=USB 
fi
if [ -f soundout.cfg ];
then
  OUTDEV=`cat soundout.cfg`
else
  OUTDEV=$INDEV
fi
# make sure there was something in the file
if [ -z ${OUTDEV} ];
then 
  OUTDEV=$INDEV
fi
#
/usr/bin/aplay -l > devices.txt
/usr/bin/aconnect 20 14
/usr/bin/jackd -R -dalsa -r48000 -n 2 -p128 -C $INDEV -P $OUTDEV