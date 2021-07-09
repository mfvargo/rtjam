# How to setup the micro SD image for the rt-sound pi

## rtjam-sound + rtjam-box unit

- burn Raspberry Pi OS Lite (32-bit) onto sd card (using imager from Raspberry Pi)
- boot unit with a monitor and keyboard (note the ip address in the boot up)
- login to console (login: pi, password: raspberry)
- run raspi-config (sudo) and enable ssh (under the Interface Options Menu)
- ssh into the unit
- sudo apt update
- sudo apt upgrade

### Install Jack

- sudo apt install jackd2  (be sure to say yes to enable realtime audio)
- to run jack do jackd -dalsa -r48000 -n 2 -p64 -Chw:USB -Phw:USB &

### Install nginx
- sudo apt install nginx


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


### TODO: Document crontab that updates


## Script to run stuff

# TODO figure out how to install the stuff 

Thinking maybe something like

```
wget localhost/pi/installer.bash
chmod +x installer.bash
sudo ./installer.bash
```
I need to write installer.bash and put it on the server.  