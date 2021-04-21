#!/bin/bash
TRIGGER=/home/pi/rtjam/TRIGGER-UPDATE
cd /home/pi/rtjam
if test -f "$TRIGGER"; then
  echo "Updating rtjam software"
  /usr/bin/systemctl stop rtjam-box
  /usr/bin/systemctl stop rtjam-sound
  /usr/bin/mv rtjam-box rtjam-box.old
  /usr/bin/mv rtjam-sound rtjam-sound.old
  /usr/bin/wget localhost/pi/rtjam-sound
  /usr/bin/wget localhost/pi/rtjam-box
  /usr/bin/chmod +x rtjam-box rtjam-sound
  /usr/bin/systemctl start rtjam-sound
  /usr/bin/systemctl start rtjam-box
  /usr/bin/rm $TRIGGER
else
  echo "No update needed"
fi
