# How to setup the micro SD image so you can build and run the rtjam software
This is to build a pi image that will let you build/run but does not have a desktop envionment. If
you really want the desktop, change the first step here to install Raspberry Pi Os with desktop.  The 
rest of the steps should be the same.

# First you just need to make a regular rtjam-unit

So a build box that will run the software needs the basic stuff for an rtjam-unit.

## OS stuff
- burn Raspberry Pi OS Lite (32-bit) onto sd card (using imager from Raspberry Pi)
- boot unit with a monitor and keyboard (note the ip address in the boot up)
- login to console (login: pi, password: raspberry)
- run raspi-config (sudo) and enable ssh (under the Interface Options Menu)
- ssh into the unit
- sudo apt update
- sudo apt upgrade
- change the password on pi account (suggest rtjamnation)
- change the hostname to something nice so you can tell see what unit you are ssh'ed into (optional)

### Change the governor to run in performance
see [/etc/rc.local](piRoot/etc/rc.local) The line
```
echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
```
is required otherwise the sound engine will generate xruns


### Install Jack (takes a few minutes)

- sudo apt install jackd2  (be sure to say yes to enable realtime audio)
- make sure the pi user is in the "audio" group so it can do realtime (groups command)

### Install nginx
nginx is used to proxy to the rtjam-nation site and to enable the fastCGI to talk to the software.

- sudo apt install nginx
- copy [default](piRoot/etc/nginx/sites-enabled/default) to /etc/nginx/sites-enabled/default
- sudo systemctl restart nginx
- sudo systemctl enable nginx

### Install fastcgi++ library
this is kind of a hack so you don't have to build it on the system.  To build requires a ton of stuff. you have to get the so from the site, mv it to /usr/lib/arm.... then make a symlink to it  (this enables rtjam-box to run)
- cd /usr/lib/arm-linux-gnueabihf
- sudo wget localhost/pi/libfastcgipp.so
- sudo ln -s libfastcgipp.so libfastcgipp.so.3

### install wiring pi
this is also kind of a hack to prevent from building on the local system.  rtjam-status required libwiringPi
- cd /usr/lib/arm-linux-gnueabihf
- sudo wget localhost/pi/libwiringPi.so.2.60
- sudo ln -s libwiringPi.so.2.60 libwiringPi.so

## Now install the app software
the next part will install the executables and the scripts needed to make them start

### system control files
systemctl is used to start/stop the rtjam pieces.  there are 4 parts.
- rtjam-jack.service (runs jackd with the correct driver)
- rtjam-box (fastcgi server to provide web apis)
- rtjam-sound (processes the audio from jack and does the network stuff)
- rtjam-status (controls the lights on the custom hardware)

the four system service files are found [here](piRoot/etc/systemd/system) and need to be copied to /etc/systemd on the pi

### update scripts and executables
These files go into a directory called /home/pi/rtjam the directory name is important as it is referred to in the system service files above so if you don't put the files in the right spot, those service won't start!
- cd (should put you into /home/pi)
- mkdir rtjam
- cd rtjam
- copy the 3 scripts from [here](piRoot/hom/pi/rtjam) to the directory.  Chmod +x on them all so they are executable
- chmod +x *.sh

### crontab entry to get new code

there is a cron job that runs every 5 minutes (as root) to see if a software update has been asked for.  Add this line to the crontab of root by doing

```
sudo crontab -e
```
and add this line
```
*/5 * * * * cd /home/pi/rtjam && /home/pi/rtjam/update-rtjam.bash
```


# build/dev stuff 
This is stuff you will need to be able to compile the softare on the pi

- sudo apt install git
- sudo apt install libgl-dev
- sudo apt install libcurl4-openssl-dev
- sudo apt-get install libasound2-dev
- sudo apt install libjack-dev

## wiring Pi
You have to build WiringPi to get the header files to build
- cd
- mkdir projects
- cd projects
- git clone https://github.com/WiringPi/WiringPi.git
- cd WiringPi
- ./build

## FastCGI stuff
FastCGI code requires cmake
- sudo apt install cmake
- directons to build it are on the git repo
```
cd ~/projects
git clone https://github.com/eddic/fastcgipp.git fastcgi++
mkdir fastcgi++.build
cd fastcgi++.build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RELEASE ../fastcgi++
make
sudo make install
```

# to build the rtjam code
```
cd ~/projects
git clone --recurse-submodules https://github.com/mfvargo/rtjam.git
cd rtjam
make
```