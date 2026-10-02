#!/bin/bash
sudo apt update
apt download kylin-printer=1.0.7kord~rc22
apt download kylin-printer=1.1.0kord-0k12~rc25

sudo apt purge kylin-printer

sudo apt install ./kylin-printer_1.0.7kord~rc22_amd64.deb
sudo apt install ./$1
ls /etc/xdg/autostart/kylin-*

sudo apt install ./kylin-printer_1.1.0kord-0k12~rc25_amd64.deb
sudo apt install ./$1
ls /etc/xdg/autostart/kylin-*