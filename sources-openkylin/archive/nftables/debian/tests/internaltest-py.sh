#!/bin/sh

# Run the internal tests of nftables (py)

set -e
cd tests/py
python3 ./nft-test.py -H
