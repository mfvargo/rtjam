# How to setup the micro SD image so you can build and run the rtjam software

This is to build a pi image that will let you build/run but does not have a desktop envionment. If
you really want the desktop, change the first step here to install Raspberry Pi Os with desktop. The
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

### Get more dev libs you will need

This is stuff you will need to be able to compile the softare on the pi

- sudo apt install git
- sudo apt install libgl-dev
- sudo apt install libcurl4-openssl-dev
- sudo apt-get install libasound2-dev
- sudo apt install cmake

```
sudo apt install git libgl-dev libcurl4-openssl-dev libasound2-dev cmake
```

###

setup you git so you can check in and it knows who you are!

```
git config --global user.name "Your Name"
git config --global user.email "youremail@yourdomain.com"
```

### Install the source code so you can copy some files!

```
cd
mkdir projects
cd ~/projects
git clone --recurse-submodules https://github.com/mfvargo/rtjam.git
```

### Install the i2s driver for the embedded sound system

This driver is used on the customer hardware. if you are building for a USB audio you can skip this step.
you have the driver without the hardware it's harmless.

- copy the [driver](doc/RPI_Sigma_I2S_Codec_Files/sigma-i2s-codec.dtbo) to /boot/overlays
- copy the config.txt

```
sudo cp ~/projects/rtjam/doc/RPi_Sigma_I2S_Codec_Files/sigma-i2s-codec.dtbo /boot/overlays/
sudo cp ~/projects/rtjam/doc/piRoot/boot/config.txt /boot
```

### Install Jack (takes a few minutes)

- sudo apt install jackd2 (be sure to say yes to enable realtime audio)
- make sure the pi user is in the "audio" group so it can do realtime (groups command)

```
sudo apt install jackd2 libjack-dev
```

Now install the jack build library

```
sudo apt install libjack-dev
```

### Install nginx

nginx is used to proxy to the rtjam-nation site and to enable the fastCGI to talk to the software.

```
sudo apt install nginx
sudo cp ~/projects/rtjam/doc/piRoot/etc/nginx/sites-enabled/default /etc/nginx/sites-enabled/default
sudo systemctl restart nginx
sudo systemctl enable nginx
```

## wiring Pi

You have to build WiringPi to get the header files to build

```
cd ~/projects
git clone https://github.com/wbeebe/WiringPi.git
cd WiringPi
./build
```

## FastCGI stuff

The FastCGI library provides bindings to the fastCGI api to nginx. This library allows rtjam-box to
get http requests from nginx to do local ops (like connect to room, change volumes, etc)

- directons to build it are on the git repo. Here is the synopsis...

```
cd ~/projects
git clone https://github.com/eddic/fastcgipp.git fastcgi++
mkdir fastcgi++.build
cd fastcgi++.build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=RELEASE ../fastcgi++
make
sudo make install
```

## Good time to reboot to have everything take effect

```
sudo reboot
```

## Build the RTJam software

to build the rtjam code

```
cd ~/projects/rtjam
make
```

## Now install the app software with make

the next part will install the executables and the scripts needed to make them start. ssh back into the box. The scripts rely on the existence of /home/pi/rtjam. Don't confuse this with /home/pi/projects/rtjam (location of the source code). So logged in as pi, make sure you mkdir rtjam so there is a place for stuff to go.

### system control files

systemctl is used to start/stop the rtjam pieces. there are 4 parts.

- rtjam-jack.service (runs jackd with the correct driver)
- rtjam-box (fastcgi server to provide web apis)
- rtjam-sound (processes the audio from jack and does the network stuff)
- rtjam-status (controls the lights on the custom hardware)

the four system service files are found [here](piRoot/etc/systemd/system) the make install-pi will copy those to the local system (/etc/systemd/system)

### update scripts and executables

These files go into a directory called /home/pi/rtjam the directory name is important as it is referred to in the system service files above so if you don't put the files in the right spot, those service won't start! The make install-pi will put files here.

## make commands to install software

Since the install has to run systemctl it needs to run as root (hence the sudo on the make commands)

```
# this will install the pi software on the local build machine
sudo make install-pi

# this will uninstall the pi software locally
sudo make uninstall-pi
```

### crontab entry to get new code

This step is only required if you want the unit to check for software updates from rtjam-nation.com/pi. There is a cron job that runs every 5 minutes (as root) to see if a software update has been asked for. Add this line to the crontab of root by doing

```
sudo crontab -e
```

and add this line

```
*/5 * * * * cd /home/pi/rtjam && /home/pi/rtjam/update-rtjam.bash
```
