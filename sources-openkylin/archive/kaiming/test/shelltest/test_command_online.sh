#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
failed_scripts=()
run_script() {
    local script_name=$1
    echo $1
    shift
    ./$script_name $@
    if [ $? -ne 0 ]; then
        failed_scripts+=($script_name)
    fi
}

#!/bin/bash

get_app_info() {
    local target_app="$1"

    local kaiming_list=$(kaiming list | tail -n +2)

    local matched=$(echo "$kaiming_list" | awk -v app="$target_app" '$1 == app')

    echo $matched
    if [ -n "$matched" ]; then
        info_output=$(kaiming info $target_app| sed 's/\x1B\[[0-9;]*[mGK]//g')
        channel=$(echo "$info_output" | awk 'NR==2 {print $3}')
        module=$(echo "$info_output" | awk 'NR==2 {print $4}')
        version=$(echo "$info_output" | awk 'NR==2 {print $5}')

        return 0
    else 
        echo "未找到应用$target_app"
        exit 1
    fi
}

run_test_case() { 
    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------1 check_search_kind--------------------------------"
    run_script check_search_kind.sh  app  $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------2 check_search_module--------------------------------"
    run_script check_search_module.sh binary  $app_name 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------3 check_install_id--------------------------------"
    run_script check_install_id.sh $app_name 
    echo "-------------------------------end--------------------------------"

    get_app_info $app_name

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------4 check_run--------------------------------"
    run_script check_run.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------5 check_run_id_version--------------------------------"
    run_script check_run_id_version.sh $app_name $version 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------6 check_run_id_version_module--------------------------------"
    run_script check_run_id_version_module.sh $app_name $version $module
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------7 check_run_id_version_module_channel--------------------------------"
    run_script check_run_id_version_module_channel.sh $app_name $version $module $channel
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------8 check_run_devel--------------------------------"
    run_script check_run_devel.sh $app_name -d
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------9 check_run_command--------------------------------"
    run_script check_run_command.sh $app_name  env
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------10 check_ps--------------------------------"
    run_script check_ps.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------11 check_enter--------------------------------"
    run_script check_enter.sh $app_name 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------12 check_enter_pid--------------------------------"
    run_script check_enter_pid.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------13 check_enter_containerid--------------------------------"
    run_script check_enter_containerid.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------14 check_enter_command-------------------------------"
    run_script check_enter_command.sh $app_name env
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------15 check_kill-------------------------------"
    run_script check_kill.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------16 check_kill_pid-------------------------------"
    run_script check_kill_pid.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------17 check_kill_containerid-------------------------------"
    run_script check_kill_containerid.sh $app_name
    echo "-------------------------------end--------------------------------"


    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------18 check_uninstall--------------------------------"
    run_script check_uninstall.sh $app_name 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------19 check_install_kind--------------------------------"
    run_script check_install_kind.sh app $app_name 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------20 check_uninstall_delete-data--------------------------------"
    run_script check_uninstall_delete-data.sh  $app_name
    echo "-------------------------------end--------------------------------"
    echo

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------21 check_install_id_version--------------------------------"
    run_script check_install_id_version.sh $app_name $version
    echo "-------------------------------end--------------------------------"
    sleep 5

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------22 check_uninstall--------------------------------"
    run_script check_uninstall.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------23 check_install_id_version_module  --------------------------------"
    run_script check_install_id_version_module.sh $app_name $version $module
    echo "-------------------------------end--------------------------------"
    sleep 10

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------24 check_uninstall--------------------------------"
    run_script check_uninstall.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------25 check_install_id--------------------------------"
    run_script check_install_id.sh $app_name 
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------26 check_list--------------------------------"
    run_script check_list.sh
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------27 check_list_kind--------------------------------"
    run_script check_list_kind.sh app
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------28 check_list_module.sh--------------------------------"
    run_script check_list_module.sh $module
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------29 check_info--------------------------------"
    run_script check_info.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------30 check_info_L.sh--------------------------------"
    run_script check_info_L.sh  $app_name -L
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------31 check_repair--------------------------------"
    run_script check_repair.sh $app_name
    echo "-------------------------------end--------------------------------"


    sudo mv /opt/kaiming/layers/stable/$arch/app/$app_name/$module/$version /opt/kaiming/layers/stable/$arch/app/$app_name/$module/0.0.1.0
    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------32 check_update--------------------------------"
    run_script check_update.sh $app_name
    echo "-------------------------------end--------------------------------"


    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------33 check_update_upgradable--------------------------------"
    run_script check_update_upgradable.sh 
    echo "-------------------------------end--------------------------------"


    base_dir="/opt/kaiming/layers/stable/x86_64/app/$app_name"  # 替换为你需要搜索的路径
    match_desktop=$(find "$base_dir" -type f -name "*.desktop" -exec grep -l "$app_name" {} \;)

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------34 check_which-------------------------------"
    run_script check_which.sh $base_dir
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------35 check_remotemodify--------------------------------"
    run_script check_remotemodify.sh https://archive2.kylinos.cn/KM/OPENKYLIN/repo
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------36 check_uninstall--------------------------------"
    run_script check_uninstall.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------37 check_install_compatibility--------------------------------"
    run_script check_install_compatibility.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------38 check_install_higher_or_lower_version_runtime--------------------------------"
    run_script check_install_higher_or_lower_version_runtime.sh top.openkylin.ukui 1.0.0
    echo "-------------------------------end--------------------------------"

    # sudo mv /opt/kaiming/layers/stable/$arch/app/$app_name/binary/1.25.0.0-ok1 /opt/kaiming/layers/stable/$arch/app/$app_name/$binary/0.0.1.0
    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------39 check_install_newversionofapp--------------------------------"
    run_script check_install_newversionofapp.sh $app_name $version
    echo "-------------------------------end--------------------------------"    

    app_id=depend.openkylin.libgsl27
    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------40 check_install_devel--------------------------------"
    run_script check_install_devel.sh $app_id
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------41 check_uninstall--------------------------------"
    echo run_script check_uninstall.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------42 check_install_f--------------------------------"
    echo "Y" | run_script check_install_f.sh $app_name
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------43 check_uninstall--------------------------------"
    echo "Y" | run_script check_uninstall.sh $app_id
    echo "-------------------------------end--------------------------------"

    echo
    echo "-------------------------------start--------------------------------"
    echo "-------------------------------44 check_uninstall--------------------------------"
    echo run_script check_uninstall.sh $app_name
    echo "-------------------------------end--------------------------------"


    if [ ${#failed_scripts[@]} -ne 0 ]; then
        echo "以下脚本执行失败:$app_name： ${failed_scripts[@]}" >> /home/$hostname/failed_ids.txt
        # exit 0
    fi
}

arch=$(uname -m)
hostname=$(whoami)

if [ -z "$1" ];then
    kaiming search --kind=app | awk 'NR>1 {print $1}' | while read -r app_name;do
        run_test_case $app_name
    done 
else
    app_name="$1"
    run_test_case $app_name
fi
