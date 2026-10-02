#!/bin/sh

# Run the internal tests of nftables (monitor)

set -e
cd tests/monitor
./run-tests.sh -d -H
