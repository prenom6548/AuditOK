#!/bin/sh

if [ "$XDG_SESSION_TYPE" != "wayland" ]; then
    bootinfo=$(cat /proc/cmdline | grep "boot=casper")
    if [ ! "$bootinfo" = "" ]; then
        echo "This is Try or install mode!"
    else
        username=`whoami`
        if [ -x "$(command -v save-param)" ]; then
            save-param -u $username
        fi
    fi
else
    echo "Session type is wayland!"
fi
