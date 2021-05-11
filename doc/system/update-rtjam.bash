#!/bin/bash
FLAGFILE=/home/pi/rtjam/update_software.flag
cd /home/pi/rtjam
# Check for update flag file
if [ -f $FLAGFILE ]; then
  echo "Updating rtjam software"
  /usr/bin/systemctl stop rtjam-box
  /usr/bin/systemctl stop rtjam-sound
  /usr/bin/rm -f /dev/mqueue/rtjamParams
  /usr/bin/rm -f /dev/shm/rtjamValues
  /usr/bin/rm -f /dev/shm/sem.rtjamValues
  /usr/bin/mv rtjam-box rtjam-box.old
  /usr/bin/mv rtjam-sound rtjam-sound.old
  /usr/bin/wget localhost/pi/rtjam-sound
  /usr/bin/wget localhost/pi/rtjam-box
  /usr/bin/chmod +x rtjam-box rtjam-sound
  /usr/bin/systemctl start rtjam-sound
  /usr/bin/systemctl start rtjam-box
else
  echo "No update needed"
fi
rm -f $FLAGFILE
