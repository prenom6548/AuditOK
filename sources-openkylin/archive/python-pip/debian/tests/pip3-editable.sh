#!/bin/sh

set -eu

export HOME=$AUTOPKGTEST_TMP

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
python3 -m pip install --break-system-packages -e .[testing]
