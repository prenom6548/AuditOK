#!/bin/sh

set -eux

export HOME=$AUTOPKGTEST_TMP/home
mkdir $HOME

python3 -m venv $AUTOPKGTEST_TMP/venv
PY=$AUTOPKGTEST_TMP/venv/bin/python3

$PY -m pip install world
$PY -m pip list --format=columns
ls -ld $AUTOPKGTEST_TMP/venv/lib/python3.*/site-packages/world-*.dist-info
$PY -m pip uninstall -y world
$PY -m pip list --format=columns
