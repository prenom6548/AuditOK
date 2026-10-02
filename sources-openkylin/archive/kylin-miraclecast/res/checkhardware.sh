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

IFNAME=$1

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

function search_p2p_dev_capabilities {
   WI_DEVICE=$1
   PHY_DEVICE=$(find_physical_for_network_interface $WI_DEVICE)
   SUCC=0
   FAIL=1

   if [ -z "$PHY_DEVICE" ]
   then
      echo "cannot find physical device for $WI_DEVICE"
      return
   fi

   if iw phy $PHY_DEVICE info | grep -Pzo "(?s)Supported interface modes.*Supported commands" | grep "P2P-device" &> /dev/null
   then
      if udevadm info --query=all --path=/sys/class/net/$WI_DEVICE | grep "ID_NET_DRIVER" | grep -E "(8821ce|88x2ce)" &> /dev/null
      then
         echo Sorry, $WI_DEVICE do not support P2P-DEVICE
      else
         echo $WI_DEVICE supports P2P-DEVICE
      fi

   else
      echo Sorry, $WI_DEVICE do not support P2P-DEVICE
      # exit 1
   fi
}

function check_pci_wnic_vendor_support {
   WI_DEVICE=$1
   if udevadm info --query=all --path=/sys/class/net/$WI_DEVICE | grep "ID_NET_DRIVER" | grep -E "(8812au|8821cu)" &> /dev/null
   then
      echo "wireless card not ok"
   else
      echo "wireless card ok"
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

RTL8821CE=$(check_pci_wnic_vendor_support $IFNAME)
echo $RTL8821CE
TARGET="not"
if [[ $RTL8821CE =~ $TARGET ]]
then
   echo "RTL8821CE not support at present"
   exit 1
fi

RETSTR=$(search_p2p_capabilities $IFNAME)
echo $RETSTR

if [[ $RETSTR =~ $TARGET ]]
then
   echo "FAILURE"
   exit 1
fi

P2PDEVSTR=$(search_p2p_dev_capabilities $IFNAME)
echo $P2PDEVSTR

if [[ $P2PDEVSTR =~ $TARGET ]]
then
   echo "network will be disabled while projection"
   exit 2
else
   echo "p2p-device support, so wifi and widi at the same time"
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

exit 0
