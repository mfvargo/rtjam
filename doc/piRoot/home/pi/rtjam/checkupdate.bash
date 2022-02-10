#!/bin/bash
WEBVER=/home/pi/rtjam/version.txt
LOCALVER=/home/pi/rtjam/version.local.txt
FLAGFILE=/home/pi/rtjam/update_software.flag
cd /home/pi/rtjam
rm $WEBVER
wget -O $WEBVER localhost/pi/version.txt
if [ "$?" -ne "0" ]; then
  echo "could not get version from server"
  exit 2
fi
cmp -s $WEBVER $LOCALVER
if [ "$?" -ne "0" ]; then
  echo "Update software" > $FLAGFILE
  echo "Update needed. Scheduled in $(( 60 - $(date +%s) % 60 )) seconds"
else
  rm -f $FLAGFILE
  echo "No update needed"
fi
