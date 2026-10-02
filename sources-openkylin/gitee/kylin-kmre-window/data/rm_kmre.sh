#!/bin/bash
# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Alan Xie    xiehuijun@kylinos.cn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# 注意：以 "[Progress]"、"[Message]" 开头的打印信息是与 "kylin-kmre-settings" 通信的信息

if [ $UID -ne 0 ]; then 
    echo "Only root user can uninstall kmre!"
    exit -1
fi

if [ $# -lt 2 ]; then
    echo "Must specify 'user' and 'uid'!"
    exit -2
fi

CALL_USER=$1
CALL_UID=$2
CLEAR_USER_DATA=$3

echo "[Progress]0"

# ------------------------------------- stop kmre container and start docker if not
RunningContainer=(`docker ps --filter name=kmre-* --format '{{.Names}}'`)
if [ -n "$RunningContainer" ]; then
    echo "[Message]Stop container: user($CALL_USER), uid($CALL_UID)"
    dbus-send --system --type=method_call --dest=cn.kylinos.Kmre /cn/kylinos/Kmre cn.kylinos.Kmre.StopContainer string:"$CALL_USER" int32:$CALL_UID

    # ------------------------------------- waiting for kmre env stopped
    if [ $? -eq 0 ]; then
        for (( i=1; i<=20; i++ ))
        do
            procs=`ps -U $CALL_USER | grep -E "kylin-kmre-manager|kylin-kmre-window|kylin-kmre-audio|kylin-kmre-sensor|kylin-kmre-gps|kylin-kmre-filewatcher|kylin-kmre-fuse" | wc -l`
            if [ $procs -eq 0 ]; then
                echo "[Message]Kmre env stopped."
                break
            elif [ $i -ge 20 ]; then
                echo "[Message]Stop kmre env timeout! Will force uninstall..."
            else
                sleep 0.5
            fi
        done
    else
        echo "[Message]Stop container failed! Will force uninstall..."
    fi
else
    echo "[Message]Container isn't running"

    masked=$(systemctl status docker | grep 'Loaded:' | grep -c masked )
    if [ $masked -gt 0 ]; then
        echo "[Message]Docker is masked, unmasked it"
        systemctl unmask docker.service
        systemctl unmask docker.socket
    fi

    running=$(systemctl status docker | grep 'Active:' | grep -c running )
    if [ $running -le 0 ]; then
        echo "[Message]Docker is not running, run it now"
        systemctl start docker
        systemctl enable docker
    fi
fi

# -------------------------------------- uninstall kmre packages
pkgs=(`dpkg -l | grep kylin-kmre | awk '{print $2}'`)
pkgs+=(
kmre
kmre-apk-installer
kmre-tools
)
#echo "kmre pkgs:${pkgs[@]}"

pkgs_num=${#pkgs[@]}

for(( i=0; i<pkgs_num; i++ )) 
do
    pkg=${pkgs[$i]}
    echo "[Message]Uninstall pkg: $pkg"
    dpkg -P --force-all $pkg
    progress=$(($((i+1))*100 / $((pkgs_num+1))))
    echo "[Progress]$progress" 
done

# --------------------------------------- remove kmre docker containers and images
echo "[Message]Remove KMRE docker containers..."
containers=(`docker ps -a --filter name=kmre-* --format '{{.Names}}'`)
for(( i=0; i<${#containers[@]}; i++ ))
do
    container=${containers[$i]}
    echo "[Message]Remove container: $container"
    docker stop $container -t 1
    docker container rm $container
done

echo "[Message]Remove KMRE docker images..."
images=(`docker images | grep kmre | awk '{print $3}'`)
for(( i=0; i<${#images[@]}; i++ ))
do
    image=${images[$i]}
    echo "[Message]Remove image: $image"
    docker rmi $image --force
done

# --------------------------------------- remove kmre env data
echo "[Message]Remove KMRE app's desktop and icon..."
JsonFilePath=/var/lib/kmre/kmre-$CALL_UID-$CALL_USER/data/local/tmp/installed.json
if [ -f $JsonFilePath ]; then
    string=`cat $JsonFilePath`
    desktop_dir=/home/$CALL_USER/.local/share/applications/
    icon_dir=/home/$CALL_USER/.local/share/icons/
    array=(${string//,/ })

    for var in ${array[@]}
    do
        list=(${var//\"/ })
        size=${#list[@]}

        for(( i=0; i<$size; i++))
        do
            name=${list[$i]}
            index=$((i + 2))

            if [[ "$name" == "appname" && $index -lt $size ]]; then
                appname=${list[$index]}
                #echo "remove $appname desktop and icon"

                desktop=$desktop_dir/$appname.desktop
                icon1=$icon_dir/$appname.svg
                icon2=$icon_dir/$appname.png
                rm -f $desktop $icon1 $icon2
            fi
        done
    done
fi

echo "[Message]Remove KMRE data..."
rm -fr /var/lib/kmre
rm -fr /usr/share/kmre

# --------------------------------------- remove kmre user config
if [ "$CLEAR_USER_DATA" = "clear" ]; then
    echo "[Message]Remove KMRE user config..."
    rm -fr /home/$CALL_USER/.config/kmre
fi

echo "[Progress]100"
echo "[Message]Uninstall kmre finished."
