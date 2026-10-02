#!/bin/bash

#judge if daemon program is running
AGT_PNUM=`ps -ef | grep 'miracle-agent' | grep -v grep | wc -l`
SRV_PNUM=`ps -ef | grep 'miracle-wifid' | grep -v grep | wc -l`

if [ $AGT_PNUM -eq 0 ] || [ $SRV_PNUM -eq 0 ]
then
    echo "miraclecast daemon not running"
    exit 2 
fi

#judge if supplicant program is running
PROG_PNUM=`ps -ef | grep 'wpa_supplicant -c' | grep -v grep | wc -l`

if [ $PROG_PNUM -gt 0  ]
then
    exit 1
fi

exit 0


