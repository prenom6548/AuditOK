#!/bin/sh

username=`whoami`
if [ -x "$(command -v save-param)" ]; then
    save-param -u $username
fi
