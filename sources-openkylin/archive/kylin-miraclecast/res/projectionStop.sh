#!/bin/bash

eval SCRIPT_DEBUG="\$$(basename $0 .sh | tr - _)_DEBUG"
SCRIPT_DEBUG=${SCRIPT_DEBUG:--1}

if [ "$SCRIPT_DEBUG" -ge 1 ]
then
   set -x
fi
if [ "$SCRIPT_DEBUG" -ge 10 ]
then
   set -v
fi

##common funcs, moved to /home/$USR/.config/miracast
LOOPBACK=772
function find_choosable_networknames {
   for i in $( ls /sys/class/net )
   do 
      if [ $( cat /sys/class/net/$i/type ) != $LOOPBACK ] 
      then
         echo $i
      fi
   done
}

function find_wireless_network_interfaces {
   for i in $( find_choosable_networknames )
   do 
      if [ -d /sys/class/net/$i/wireless ]
      then
         echo $i
      fi
   done
}
##END of common funcs, moved to /home/$USR/.config/miracast

WIFI_COUNT=0
WIFI_NAMES="$(find_wireless_network_interfaces)"
if [ -n "$WIFI_NAMES" ]
then
  WIFI_COUNT=$(echo "$WIFI_NAMES" | wc -l)
fi

if [ 0 = $WIFI_COUNT ]
then
   echo There is no wireless devices available
   exit 1
elif [ 1 = $WIFI_COUNT ]
then
   WIFI_NAME="$WIFI_NAMES"
elif [ 2 -ge $WIFI_COUNT ]
then
   echo Support Only one wireless card @v1.0
fi

systemctl stop miracle-wifid.service
nmcli device set $WIFI_NAME managed yes
