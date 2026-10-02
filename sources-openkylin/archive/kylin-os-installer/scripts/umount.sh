#!/bin/bash
sudo swapoff -a || true
sudo umount /target/run || true
sudo umount /target/sys || true
sudo umount /target/proc || true
sudo umount /target/cdrom || true
sudo umount /target/dev/pts || true
sudo umount /target/dev/ || true
sudo umount /target/boot/efi || true
sudo umount /target/ || true
