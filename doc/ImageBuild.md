# How to setup the micro SD image for the rt-sound pi

## rtjam-sound + rtjam-box unit

- burn Raspberry Pi OS Lite (32-bit) onto sd card (using imager from Raspberry Pi)
- boot unit with a monitor and keyboard (note the ip address in the boot up)
- run raspi-config (sudo) and enable ssh
- ssh into the unit
- sudo apt update
- sudo apt upgrade

### Install Jack

- sudo apt install jackd2  (be sure to say yes to enable realtime audio)
- to run jack do jackd -dalsa -r48000 -n 2 -p64 -Chw:USB -Phw:USB &

### Install nginx
- sudo apt install nginx

### TODO: Document crontab that updates


## Script to run stuff

runme.bash
```
sudo sh -c "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
jackd -dalsa -r48000 -n 2 -p64 -Chw:USB -Phw:USB &
./rtjam-sound &
./rtjam-box &
```

service file for /etc/systemd/system (sudo systemctl restart rtjam.service)
```
[Unit]
Description=rtjam service
After=network.target

[Service]
ExecStart=/usr/bin/bash runme.bash
WorkingDirectory=/home/pi/rtjam
StandardOutput=inherit
StandardError=inherit
Restart=always
User=pi

[Install]
WantedBy=multi-user.target
```
