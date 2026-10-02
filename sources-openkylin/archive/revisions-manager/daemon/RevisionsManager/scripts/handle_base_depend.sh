#!/bin/bash


debCount=2
dependDebInfo_0_name="libkysdk-qtwidgets"
dependDebInfo_0_ver="2.3.1.0-0k0.1"
dependDebInfo_1_name="libkysdk-waylandhelper"
dependDebInfo_1_ver="2.3.1.0-0k0.1"

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
        echo "install $debList start."
        apt install -y ${debList}
    fi
else
    echo "$compatDep_path not exists."
fi


