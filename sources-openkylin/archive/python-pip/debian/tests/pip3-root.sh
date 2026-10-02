#!/bin/sh

set -eux


python3 -m pip install --break-system-packages world
python3 -m pip list --format=columns
ls -ld /usr/local/lib/python3.*/dist-packages/world-*.dist-info
python3 -m pip uninstall --break-system-packages -y world
python3 -m pip list --format=columns
