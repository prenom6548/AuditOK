#!/bin/bash

#judge if frontend program is running
PROG_PNUM=`ps -ef | grep 'miraclecast-frontend' | grep -v grep | wc -l`

if [ $PROG_PNUM -gt 0  ]
then
    echo "miracast frontend is running"
    exit 1
else
    echo "miracast frontend not running" 
fi


