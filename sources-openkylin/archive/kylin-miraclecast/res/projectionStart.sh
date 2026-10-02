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

##START of common funcs, moved to /home/$USR/.config/miracast latter
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

function find_physical_for_network_interface {
   PHY_INDEX=$(iw dev $1 info | grep wiphy | awk '{print $2}')
   if [ -n "$PHY_INDEX" ]
   then
      echo phy$PHY_INDEX
   fi
}

function search_p2p_capabilities {
   WI_DEVICE=$1
   PHY_DEVICE=$(find_physical_for_network_interface $WI_DEVICE)
   SUCC=0
   FAIL=1

   if [ -z "$PHY_DEVICE" ]
   then
      echo "cannot find physical device for $WI_DEVICE"
      return
   fi

   if iw phy $PHY_DEVICE info | grep -Pzo "(?s)Supported interface modes.*Supported commands" | grep "P2P" &> /dev/null
   then
      echo $WI_DEVICE supports P2P
   else
      echo Sorry, $WI_DEVICE do not support P2P
      # exit 1
   fi
}
##END of common funcs, moved to /home/$USR/.config/miracast latter

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

RETSTR=$(search_p2p_capabilities $WIFI_NAME)
echo $RETSTR
TARGET="not"

if [[ $RETSTR =~ $TARGET ]]
then
   echo "FAILURE"
   exit 1
fi

AGENT_PNAME="miracle-agent"
AGENT_PNUM=`ps -ef | grep $AGENT_PNAME | grep -v grep | wc -l`
if [ $AGENT_PNUM -gt 0  ]
then
   echo "SUCCESS"
else
   echo "Agent not running!!! Quit..."
   exit 1
fi

nmcli device set $WIFI_NAME managed no
systemctl restart miracle-wifid.service