#!/bin/bash

if cat /proc/cmdline | grep -q "boot=casper\|boot=live"; then
    echo "kylin-core-dump-monitor is not started in live mode"
    exit 1
fi

echo "kylin-core-dump-monitor is started in normal mode"
exit 0