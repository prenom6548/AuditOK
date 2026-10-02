#!/bin/bash


debCount=10
dependDebInfo_0_name="kare" 
dependDebInfo_0_ver="2.0-0ok2update3"
dependDebInfo_1_name="runc"
dependDebInfo_1_ver="1.1.0-ok1"
dependDebInfo_2_name="containerd"
dependDebInfo_2_ver="1.5.9-ok6"
dependDebInfo_3_name="docker.io"
dependDebInfo_3_ver="20.10.16-ok1"
dependDebInfo_4_name="bubblewrap"
dependDebInfo_4_ver="0.4.0-ok3"
dependDebInfo_5_name="iptables"
dependDebInfo_5_ver="1.8.4-ok2"
dependDebInfo_6_name="libip4tc2"
dependDebInfo_6_ver="1.8.4-ok2"
dependDebInfo_7_name="libip6tc2"
dependDebInfo_7_ver="1.8.4-ok2"
dependDebInfo_8_name="libxtables12"
dependDebInfo_8_ver="1.8.4-ok2"
dependDebInfo_9_name="debootstrap"
dependDebInfo_9_ver="1.0.118-ok4"


debList=""

function get_depends(){
    for ((i=0;i<$debCount;i++))
    do
        devNameStr="dependDebInfo_"$i"_name"
        debName=${!devNameStr}
        debVerStr="dependDebInfo_"$i"_ver"
        debVer=${!debVerStr}
        curVer=`dpkg -l | grep $debName | awk '{print $3}'`

        echo $debName $debVer $curVer

        if [ -n "$curVer" ]; then
            ret=`dpkg --compare-versions $curVer ge $debVer && echo "yes"`
            if [ "$ret" = "yes" ]; then
                continue
            fi
        fi

        debList=$debList"./"$debName"*.deb " 
    done
    echo $debList
}

arch=`uname -m`
compatDep_path="/usr/share/revisions-manager/data/depends/$arch/"
if [ -e $compatDep_path ]; then
    get_depends
    cd $compatDep_path
    if [ -n "$debList" ];then
        apt install -y ${debList}
    fi
else
    echo "$compatDep_path not exists."
fi


