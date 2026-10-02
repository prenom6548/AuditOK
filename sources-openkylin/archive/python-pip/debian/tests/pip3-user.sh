#!/bin/sh

set -eux

export HOME=$AUTOPKGTEST_TMP
export PATH=$PATH:$HOME/.local/bin

if [ $(id -u) = 0 ]
then
    adduser --quiet --system --group --no-create-home testing
    user=testing
    mkdir $HOME/.cache
    chown -R $user $HOME
    runuser="runuser -p -u $user --"
else
    runuser=""
fi

$runuser python3 -m pip install --break-system-packages world
$runuser python3 -m pip list --format=columns
$runuser python3 -m pip show world
ls -ld $HOME/.local/lib/python3.*/site-packages/world-*.dist-info
$runuser python3 -m pip uninstall --break-system-packages -y world
$runuser python3 -m pip list --format=columns
$runuser python3 -m pip list --outdated
if [ $(id -u) = 0 ]
then
    deluser --quiet testing
fi
