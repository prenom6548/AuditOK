#!/bin/sh

# Run the internal tests of nftables (json_echo)

set -e
cd tests/json_echo
python3 ./run-test.py -H
