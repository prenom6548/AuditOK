#!/bin/sh

# Run the internal tests of nftables (shell)

set -e
cd tests/shell
NFT=$(which nft) ./run-tests.sh -v
