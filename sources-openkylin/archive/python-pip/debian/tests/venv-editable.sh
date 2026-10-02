#!/bin/sh

set -eux

export HOME=$AUTOPKGTEST_TMP/home
mkdir $HOME

python3 -m venv $AUTOPKGTEST_TMP/venv
PY=$AUTOPKGTEST_TMP/venv/bin/python3

cd $HOME
mkdir stupid
cat > stupid/setup.py <<EOF
from setuptools import setup

setup(
    name='stupid',
    version='0.1',
    extras_require = {
        'testing': [],
        }
    )
EOF

cd stupid

$PY -m pip install -e .[testing]
