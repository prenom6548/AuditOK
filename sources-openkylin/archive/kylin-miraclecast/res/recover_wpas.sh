#!/bin/bash

sudo systemctl unmask wpa_supplicant.service
sudo systemctl start wpa_supplicant.service
echo "We recover wpa_supplicant.service !!!!!!!!!!!!!!!"

