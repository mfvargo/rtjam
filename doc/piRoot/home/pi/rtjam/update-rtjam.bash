#!/bin/bash
FLAGFILE=/home/pi/rtjam/update_software.flag
cd /home/pi/rtjam
# Check for update flag file
if [ -f $FLAGFILE ]; then
  echo "Updating rtjam software"
  /usr/bin/systemctl stop rtjam-box
  /usr/bin/systemctl stop rtjam-sound
  /usr/bin/systemctl stop rtjam-status
  /usr/bin/rm -f /dev/mqueue/rtjamParams
  /usr/bin/rm -f /dev/shm/rtjamValues
  /usr/bin/rm -f /dev/shm/sem.rtjamValues
  /usr/bin/rm -f /dev/shm/rtjamLightSettings
  /usr/bin/rm -f /dev/shm/sem.rtjamLightSettings
  /usr/bin/mv rtjam-box rtjam-box.old
  /usr/bin/mv rtjam-sound rtjam-sound.old
  /usr/bin/mv rtjam-status rtjam-status.old
  /usr/bin/mv rtjam-midi rtjam-midi.old
  /usr/bin/wget localhost/pi/rtjam-status
  /usr/bin/wget localhost/pi/rtjam-sound
  /usr/bin/wget localhost/pi/rtjam-box
  /usr/bin/wget localhost/pi/rtjam-midi
  /usr/bin/chmod +x rtjam-box rtjam-sound rtjam-status rtjam-midi
  /usr/bin/systemctl start rtjam-status
  /usr/bin/systemctl start rtjam-sound
  /usr/bin/systemctl start rtjam-box
else
  echo "No update needed"
fi
rm -f $FLAGFILE
