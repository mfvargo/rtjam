#!/bin/bash
WEBVER=/home/pi/rtjam/version.txt
LOCALVER=/home/pi/rtjam/version.local.txt
cd /home/pi/rtjam
rm $WEBVER
wget -O $WEBVER localhost/pi/version.txt
if [ "$?" -ne "0" ]; then
  echo "could not get version from server"
  exit 2
fi
cmp -s $WEBVER $LOCALVER
if [ "$?" -ne "0" ]; then
  echo "Updating rtjam software"
  /usr/bin/systemctl stop rtjam-broadcast
  /usr/bin/mv rtjam-broadcast rtjam-broadcast.old
  /usr/bin/wget localhost/pi/rtjam-broadcast
  /usr/bin/chmod +x rtjam-broadcast
  /usr/bin/systemctl start rtjam-broadcast
else
  echo "No update needed"
fi
