#!/bin/bash

set -e
set -o pipefail

export info_version_string=
export mate_data_add_string=
export os_version_name=
export sign_position=

mkdir -p /var/log/installer

# 拷贝配置文件
do_copy_config() {

  # sp1 使用 automatic
  if grep -q automatic /proc/cmdline; then
    if grep -q "url=" /proc/cmdline; then
        if [[ -f /cdrom/ky-installer.cfg ]]; then
                cp /cdrom/ky-installer.cfg /tmp
        fi
        if grep -q "cfg=" /proc/cmdline; then
                cmd_cfg=$(cat /proc/cmdline)
                url_ip=$(echo ${cmd_cfg##*url=}| awk '{print $1}')
                url_ip=${url_ip%/*}
                cfg=$(echo ${cmd_cfg##*cfg=}| awk '{print $1}')
                wget ${url_ip}/${cfg} -O /tmp/ky-installer.cfg
        fi
    else
        if grep -q "cfg=" /proc/cmdline; then
                cmd_cfg=$(cat /proc/cmdline)
                cfg=$(echo ${cmd_cfg##*cfg=}| awk '{print $1}')
                cp /cdrom/${cfg}  /tmp/ky-installer.cfg
        else
                if [[ -f /cdrom/ky-installer.cfg ]]; then
                        cp /cdrom/ky-installer.cfg /tmp
                elif [[ -f /cdrom/kyple-installer.cfg ]]; then
                        cp /cdrom/kyple-installer.cfg /tmp/ky-installer.cfg
                else
                        echo "ky-installer.cfg does not exist"
                        echo "您现在用的是自动安装流程，请将ky-installer.cfg放到iso根目录下"
                        exit 1
                fi

        fi
    fi
  # 990/9A0 使用此设置，ple-mode 审核模式，test-mode 全自动安装模式
  elif grep -qE 'ple-mode|test-mode' /proc/cmdline; then
    [[ -f /cdrom/kyple-installer.cfg ]] && cp /cdrom/kyple-installer.cfg /tmp/ky-installer.cfg
  else
    cp /var/log/installer/ky-installer.cfg /tmp
  fi
  echo "ky-installer.cfg exist"
  if [[ -f /tmp/ky-installer.cfg ]]; then
        cp /tmp/ky-installer.cfg /var/log/installer/ky-installer.cfg || echo "拷贝/tmp/ky-installer.cfg失败"
  fi
}

do_copy_config >/var/log/installer/kylin-os-installer-detail.log


# 使用绝对路径，方便在 /target 中加载
cp utils /tmp
. /tmp/utils

echo "installer-progress=0" >${INSTALLER_PROGRESS}
echo "installer-progress-next=3" >>${INSTALLER_PROGRESS}

case ${LANGUAGES} in
        'zh_CN')
                echo "系统安装前准备..." >${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Preparation before system installation..." >${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "མ་ལག་སྒྲག་སྦྱར་གོང་གི་གྲ་སྒྲག།..." >${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "系統安裝前準備..." >${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠢᠰᠲ᠋ᠡᠮ ᠲᠣᠨᠣᠭᠯᠠᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ ᠪᠡᠯᠡᠳᠬᠡᠯ..." >${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "系统安装前准备..." >${LOG_PROGRESS_FILE}
                ;;
esac

if grep -q automatic /proc/cmdline; then
        data_unformat=$(get_value data-unformat)
        if [[ "${data_unformat}" == "true" ]]; then
                save_disk=$(get_value devpath)
                if ! grep -q "save-data=${save_disk}" /var/log/installer/os-prober.log; then
                        echo "您选择了保留用户数据安装，但机器系统不满足保留用户数据要求，不能进行保留用户数据安装" >>/var/log/installer/kylin-os-installer.log
                        exit 1
                fi
        fi
fi

# 运行 prepare 脚本
do_prepare() {
  mkdir -p /var/log/installer

  if [[ -d /cdrom/hooks/prepare ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行安装前prepare定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Customize prepare scripts before installation execution..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་སྒྲག་སྦྱར་གོང་prepareངེས་སྲོལ་འཁྲབ་དེབ། ..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行安裝前prepare定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ prepare ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行安装前prepare定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /cdrom/hooks/prepare || do_error hooks-prepare
    case ${LANGUAGES} in
        'zh_CN')
                echo "安装前prepare定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Prepare custom script execution before installation completed" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "སྒྲག་སྦྱར་གོང་prepareངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "安裝前prepare定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡ prepare ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠵᠦ ᠳᠠᠭᠤᠰᠭᠠᠪᠠ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "安装前prepare定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
  fi
  hooks=$(get_value hooks)
  if [[ -n $hooks ]]; then
#  if grep -q "hooks=" /proc/cmdline; then
    if grep -q "url=" /proc/cmdline; then
            cmd_cfg=$(cat /proc/cmdline)
            url_ip=$(echo ${cmd_cfg##*url=}| awk '{print $1}')
            url_ip=${url_ip%/*}
#            hooks=$(echo ${cmd_cfg##*hooks=}| awk '{print $1}')
            wget -P/tmp ${url_ip}/${hooks}
            tar -xf /tmp/${hooks} -C /tmp/
    fi
  fi

  if [[ -d /tmp/hooks/prepare ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行安装前prepare定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Customize prepare scripts before installation execution..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་སྒྲག་སྦྱར་གོང་prepareངེས་སྲོལ་འཁྲབ་དེབ། ..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行安裝前prepare定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ prepare ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行安装前prepare定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /tmp/hooks/prepare || do_error hooks-prepare
    case ${LANGUAGES} in
        'zh_CN')
                echo "安装前prepare定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Prepare custom script execution before installation completed" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "སྒྲག་སྦྱར་གོང་prepareངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "安裝前prepare定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡ prepare ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠵᠦ ᠳᠠᠭᠤᠰᠭᠠᠪᠠ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "安装前prepare定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
  fi

  run_scripts_dir prepare

}



# 运行 in_chroot 脚本
do_run_target() {
  new_sysroot_dir=$(get_value new-root)
  mkdir -p $new_sysroot_dir/${IN_TARGET}
  cp utils /tmp/ky-installer.cfg $new_sysroot_dir/tmp
  cp -rf k.sh in_chroot $new_sysroot_dir/${IN_TARGET}
  
  if [[ -d /tmp/hooks/in_chroot ]]; then
     cp -r /tmp/hooks/in_chroot $new_sysroot_dir/tmp
  fi

  chroot $new_sysroot_dir bash ${IN_TARGET}/k.sh >>${LOG_FILE} 2>&1
}

# 运行 after 脚本
do_after() {
  run_scripts_dir after

  if [[ -d /cdrom/hooks/after ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行安装后after定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Execute customized script after installation..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་སྒྲག་སྦྱར་རྗེས་afterངེས་སྲོལ་འཁྲབ་དེབ།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行安裝後after定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠶᠢ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠦᠭᠰᠡᠨ ᠦ ᠳᠠᠷᠠᠭ<180e>ᠠ after ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠦᠢᠯᠡᠳᠬᠦ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行安装后after定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /cdrom/hooks/after || do_error hooks-after
    case ${LANGUAGES} in
        'zh_CN')
                echo "安装后after定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Custom script execution is completed after installation" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "སྒྲག་སྦྱར་རྗེས་afterངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "安裝後after定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠭᠰᠠᠨ ᠤ ᠳᠠᠷᠠᠭᠠ after ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠦᠢᠯᠡᠳᠬᠦ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "安装后after定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
  fi

  if [[ -d /tmp/hooks/after ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行安装后after定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Execute customized script after installation..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་སྒྲག་སྦྱར་རྗེས་afterངེས་སྲོལ་འཁྲབ་དེབ།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行安裝後after定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠬᠤ ᠶᠢ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠦᠭᠰᠡᠨ ᠦ ᠳᠠᠷᠠᠭ<180e>ᠠ after ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠦᠢᠯᠡᠳᠬᠦ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行安装后after定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /tmp/hooks/after || do_error hooks-after
    case ${LANGUAGES} in
        'zh_CN')
                echo "安装后after定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Custom script execution is completed after installation" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "སྒྲག་སྦྱར་རྗེས་afterངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "安裝後after定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠰᠠᠭᠤᠯᠭᠠᠭᠰᠠᠨ ᠤ ᠳᠠᠷᠠᠭᠠ after ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠦᠢᠯᠡᠳᠬᠦ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "安装后after定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
  fi
}

# chroot卸载对应的包，目前废弃
do_in_chroot_remove_deb() {
	new_sysroot_dir=$(get_value new-root)
	umount $new_sysroot_dir/usr
	umount $new_sysroot_dir/etc
	umount $new_sysroot_dir/var
	mkdir -p $new_sysroot_dir${IN_TARGET}
  	cp utils /tmp/ky-installer.cfg $new_sysroot_dir/tmp
	cp -rf remove-deb.sh in_chroot_remove_deb $new_sysroot_dir${IN_TARGET}
	chroot $new_sysroot_dir bash ${IN_TARGET}/remove-deb.sh
	echo "run in chroot remove deb 结束" >> ${LOG_FILE} 2>&1
}

# 拷贝overlay数据至备份还原点
do_copy_overlay_data_backup_deploy() {
	# 获取俩个deploy节点的hash值
	deploy_hashvalue_root=$(get_value deploy_hashvalue_root)
	deploy_hashvalue_backup=$(get_value deploy_hashvalue_backup)
	[ -d /target/ostree/pkgs/ovl-$deploy_hashvalue_backup ] || mkdir -p /target/ostree/pkgs/ovl-$deploy_hashvalue_backup
	rsync -a /target/ostree/pkgs/ovl-$deploy_hashvalue_root/* /target/ostree/pkgs/ovl-$deploy_hashvalue_backup/
	echo "拷贝overlayfs 数据完成" >> $LOG_FILE 2>&1
}

# 修改根文件系统权限
do_chown_sysroot_permissions() {
	os_version_name=$(get_value os_version_name)
    for sysroot_dir_file in /target/ostree/deploy/$os_version_name/deploy/*; do
    if echo $sysroot_dir_file | grep -v "origin" > /dev/null; then
        chattr +i $sysroot_dir_file
    fi
	done
}

# 执行 post 脚本
do_cdrom_post_actions() {

  case ${LANGUAGES} in
        'zh_CN')
                echo "执行postactions 脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Execute postactions script..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་postactionsའཁྲབ་དེབ།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行postactions腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "postactions ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠬᠦ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行postactions 脚本..." >>${LOG_PROGRESS_FILE}
                ;;
  esac

  new_sysroot_dir=$(get_value new-root)
  if [[ -f /cdrom/.kylin-post-actions ]] && [[ "${is_ghost}" != "true" ]]; then
    cp /cdrom/.kylin-post-actions $new_sysroot_dir/${IN_TARGET}
    chroot $new_sysroot_dir bash ${IN_TARGET}/.kylin-post-actions >>${LOG_FILE} 2>&1 || do_error post-actions
  fi

  if [[ -f /cdrom/.kylin-post-actions-nochroot ]] && [[ "${is_ghost}" != "true" ]]; then
    bash /cdrom/.kylin-post-actions-nochroot >>${LOG_FILE} 2>&1 || do_error post-actions-nochroot
  fi

  if [[ "${is_ghost}" == "true" ]]; then
    tmp_path="${new_sysroot_dir}/boot/loader.0/grub.cfg"
    tmp_conf_path="${new_sysroot_dir}/boot/loader.0/entries/"
    if [ -f ${tmp_path} ]; then
        new_sysroot_part=$(get_value part-root)
        root_uuid=`lsblk -o UUID ${new_sysroot_part} | grep -v UUID`
        old_root_uuid=`cat ${tmp_path} | grep -m 1 "root=UUID="`
        old_root_uuid=$(echo ${old_root_uuid##*"root=UUID="}| awk '{print $1}')
        sed -i 's/'"${old_root_uuid}"'/'"${root_uuid}"'/' ${tmp_path} || echo "set grub.cfg root-uuid failed"
	
	if [ -d ${tmp_conf_path} ]; then
		conf_file=`find "${tmp_conf_path}" | grep "ostree"`
		if [[ -n ${conf_file} ]]; then
        		for conf in ${conf_file}; do
				sed -i 's/'"${old_root_uuid}"'/'"${root_uuid}"'/' "${conf}" || echo "set ${conf} root-uuid failed"
			done
		fi
	fi
    fi
  fi

  case ${LANGUAGES} in
        'zh_CN')
                echo "postactions 脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Postactions script execution completed" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "Postactionsའཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "postactions腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "postactions ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠵᠦ ᠳᠠᠭᠤᠰᠭᠠᠪᠠ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "postactions 脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
  esac
}

# 拷贝日志
do_copy_log() {
  new_sysroot_dir=$(get_value new-root)
  msg "正在拷贝安装日志..."
  cp -r /var/log/installer/ $new_sysroot_dir/var/log/
  
  if [[ -f $new_sysroot_dir/var/log/installer/ky-installer.cfg ]]; then
	if [[ "${is_oem_mode}" = "false" ]]; then
  		sed -i "/password/d" $new_sysroot_dir/var/log/installer/ky-installer.cfg || true
  		sed -i "/encryptyPWD/d" $new_sysroot_dir/var/log/installer/ky-installer.cfg || true
  	fi
  fi
  if grep -q 'auti-mode' /proc/cmdline; then
        if grep -q 'auti-only' /proc/cmdline; then
        	sed -i "/auti-only/d" $new_sysroot_dir/var/log/installer/ky-installer.cfg
        	echo "auti-only=true" >>$new_sysroot_dir/var/log/installer/ky-installer.cfg
        fi
  fi


}

#离线包升级更新
do_update_installer() {
    kpb_list=$(find /cdrom/patches/ | grep .kpb ) || true
    new_sysroot_dir=$(get_value new-root)
    if [[ -n ${kpb_list} ]]; then
        for kpb_one in $kpb_list
        do
                mkdir -p $new_sysroot_dir/patches
                cp -a ${kpb_one} $new_sysroot_dir/patches
                mkdir -p $new_sysroot_dir/media/OfflineSource
                target_kpb="$new_sysroot_dir/patches/*.kpb"
                mount  ${target_kpb} $new_sysroot_dir/media/OfflineSource
                if [ -e $new_sysroot_dir/media/OfflineSource/update-install.py ]; then
                        echo "start exec update-install.py"
                        echo ${target_kpb}
                        chroot $new_sysroot_dir /bin/sh -c "python3 /media/OfflineSource/update-install.py" || do_error kbp
                fi
                umount -l $new_sysroot_dir/media/OfflineSource
                rm -rf $new_sysroot_dir/patches
                rm -rf $new_sysroot_dir/media/OfflineSource
        done
    else
        if grep -q "url=" /proc/cmdline; then
                kbp_patches=$(get_value patches)
                if [[ -n $kbp_patches ]]; then
                        cmd_cfg=$(cat /proc/cmdline)
                        url_ip=$(echo ${cmd_cfg##*url=}| awk '{print $1}')
                        url_ip=${url_ip%/*}
                        mkdir -p $new_sysroot_dir/patches
                        wget -P$new_sysroot_dir/patches ${url_ip}/${kbp_patches}
                        tar -xf $new_sysroot_dir/patches/${kbp_patches} -C $new_sysroot_dir/patches/
                        target_kpb=`find $new_sysroot_dir/patches | grep .kpb` || true
                        for kpb_two in $target_kpb
                        do
                                mkdir -p $new_sysroot_dir/media/OfflineSource
                                mount  ${kpb_two} $new_sysroot_dir/media/OfflineSource
                                if [ -e $new_sysroot_dir/media/OfflineSource/update-install.py ]; then
                                        echo "start exec update-install.py"
                                        chroot $new_sysroot_dir /bin/sh -c "python3 /media/OfflineSource/update-install.py" || do_error kbp
                                fi
                                umount -l $new_sysroot_dir/media/OfflineSource
                                rm -rf $new_sysroot_dir/patches
                                rm -rf $new_sysroot_dir/media/OfflineSource
                        done
                fi
        fi
    fi
}

do_del_auti_mode() {
  new_sysroot_dir=$(get_value new-root)
  os_version_name=$(get_value os_version_name)
  deploy_hashvalue_root=$(get_value deploy_hashvalue_root)
  deploy_ovl_root=/target/ostree/pkgs/ovl-$deploy_hashvalue_root
  deploy_hashvalue_root=/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue_root
  echo "deploy_ovl_root=$deploy_ovl_root" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
  echo "deploy_hashvalue_root=$deploy_hashvalue_root" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
 
  if [[ -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/oem-qc.desktop ]]; then
	rm -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/oem-qc.desktop
  fi
  
  if [[ -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/oem-unlock.desktop ]]; then
	rm -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/oem-unlock.desktop
  fi

  if [[ -d $deploy_ovl_root/usr-ovl/usr-lower/share/oem-test ]]; then
       mv $deploy_ovl_root/usr-ovl/usr-lower/share/oem-test /usr/share/
  fi


  deploy_hashvalue_backup=$(get_value deploy_hashvalue_backup)
  deploy_ovl_backup=/target/ostree/pkgs/ovl-$deploy_hashvalue_backup
  deploy_hashvalue_backup=/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue_backup
  backup_desktop_dir=$deploy_ovl_backup/etc-ovl/etc-lower/xdg/autostart
  
  echo "deploy_ovl_backup=$deploy_ovl_backup" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
  echo "deploy_hashvalue_backup=$deploy_hashvalue_backup" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log

  if [[ -f $deploy_hashvalue_backup/etc/xdg/autostart/oem-qc.desktop ]]; then
        rm -f $deploy_hashvalue_backup/etc/xdg/autostart/oem-qc.desktop
  fi
  if [[ -f $deploy_ovl_backup/etc-ovl/etc-lower/xdg/autostart/oem-qc.desktop ]]; then
	rm -f $deploy_ovl_backup/etc-ovl/etc-lower/xdg/autostart/oem-qc.desktop
  fi

  if [[ -f $deploy_hashvalue_backup/usr/sbin/oem-second ]]; then
        rm -f $deploy_hashvalue_backup/usr/sbin/oem-second
  fi

  if [[ -f $deploy_hashvalue_backup/etc/xdg/autostart/oem-unlock.desktop ]]; then
        rm -f $deploy_hashvalue_backup/etc/xdg/autostart/oem-unlock.desktop
  fi
  if [[ -f $deploy_ovl_backup/etc-ovl/etc-lower/xdg/autostart/oem-unlock.desktop ]]; then
	rm -f $deploy_ovl_backup/etc-ovl/etc-lower/xdg/autostart/oem-unlock.desktop
  fi

  if [[ -d $deploy_ovl_backup/usr-ovl/usr-lower/share/oem-test ]]; then
	rm -rf $deploy_ovl_backup/usr-ovl/usr-lower/share/oem-test 
  fi

  if grep -q 'test-mode' /proc/cmdline; then
    oem_mode="oem-automatic"
  else
    oem_mode="oem"
  fi

  if [[ -f $new_sysroot_dir/usr/share/applications/kylin-os-installer.desktop ]]; then
  	cp -a $new_sysroot_dir/usr/share/applications/kylin-os-installer.desktop $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/ || echo "kylin-os-installer.desktop is unexit" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
  	sed -i "/Exec/d" $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/kylin-os-installer.desktop
  	echo "Exec=/usr/bin/kylin-os-installer-${oem_mode}" >>$deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/kylin-os-installer.desktop
	if [[ -d $backup_desktop_dir ]]; then
		cp -a $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/kylin-os-installer.desktop $backup_desktop_dir
	fi
  else
	echo "kylin-os-installer.desktop is unexit" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
  fi

}

do_auti_mode() {

  new_sysroot_dir=$(get_value new-root)
  os_version_name=$(get_value os_version_name)
  deploy_hashvalue_root=$(get_value deploy_hashvalue_root)
  deploy_ovl_root=/target/ostree/pkgs/ovl-$deploy_hashvalue_root
  deploy_hashvalue_root=/target/ostree/deploy/$os_version_name/deploy/$deploy_hashvalue_root
  
  if [[ -d /usr/share/oem-test ]]; then
       /usr/bin/rsync -a  /usr/share/oem-test/  $deploy_ovl_root/usr-ovl/usr-lower/share/oem-test
  fi
  
  if [[ -f /usr/share/kylin-os-installer/data/oem-qc.desktop ]]; then
       cp -a /usr/share/kylin-os-installer/data/oem-qc.desktop $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/
       cp -a /usr/share/kylin-os-installer/data/oem-qc $new_sysroot_dir/var/log/installer/oem-qc

       if grep -q 'auti-only' /proc/cmdline; then
              cp -a $new_sysroot_dir/usr/share/kylin-os-installer/scripts/oem-second $new_sysroot_dir/usr/sbin/
              if [[ -f /cdrom/ky-installer.cfg ]]; then
                      cp /cdrom/ky-installer.cfg $new_sysroot_dir/usr/share/kylin-os-installer/ky-installer.cfg
              fi
       else
              cp -a $new_sysroot_dir/usr/share/kylin-os-installer/scripts/oem-second $new_sysroot_dir/usr/sbin/
       fi
   fi

   if [[ -f /usr/share/kylin-os-installer/data/oem-unlock.desktop ]]; then
              cp -a /usr/share/kylin-os-installer/data/oem-unlock.desktop $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/
              cp -a /usr/share/kylin-os-installer/data/oem-unlock $new_sysroot_dir/var/log/installer/oem-unlock
   fi

   if [[ -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/kylin-os-installer.desktop ]]; then
	rm -f $deploy_ovl_root/etc-ovl/etc-lower/xdg/autostart/kylin-os-installer.desktop
	
   fi

   if [[ -f $new_sysroot_dir/usr/share/lightdm/lightdm.conf.d/96-kylin-os-installer.conf ]]; then
        mv $new_sysroot_dir/usr/share/lightdm/lightdm.conf.d/96-kylin-os-installer.conf $new_sysroot_dir/usr/share/kylin-os-installer
   fi


}

# 出厂备份
do_factory_backup() {

  case ${LANGUAGES} in
        'zh_CN')
                echo "正在进行系统出厂备份..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "System factory backup in progress..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "བཞིན་ལ་མ་ལག་ནས་ཐོན་པའི་备份། ..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "正在進行系統出廠備份..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠶᠠᠭ ᠰᠢᠰᠲ᠋ᠡᠮ ᠤᠨ ᠦᠢᠯᠡᠳᠪᠦᠷᠢ ᠡᠴᠡ ᠭᠠᠷᠭᠠᠬᠤ ᠨᠦᠬᠡᠴᠡᠯᠡᠯ ᠢ ᠶᠠᠪᠤᠭᠳᠠᠭᠤᠯᠵᠤ ᠪᠠᠶᠢᠨᠠ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "正在进行系统出厂备份..." >>${LOG_PROGRESS_FILE}
                ;;
  esac


  echo "installer-progress=96" >${INSTALLER_PROGRESS}
  echo "installer-progress-next=99" >>${INSTALLER_PROGRESS}
  #删除出厂备份中的审核模式
  if grep -q 'auti-mode' /proc/cmdline; then
        do_del_auti_mode
  fi

  #umount /target/backup
  msg "正在进行系统备份..."
  new_sysroot_dir=$(get_value new-root)
  deploy_hashvalue_backup=$(get_value deploy_hashvalue_backup)
  ## 工厂备份
  /usr/bin/backup-factory --factorybackup $new_sysroot_dir $deploy_hashvalue_backup || true
  msg "系统备份完成"

  
  #还原出厂备份中的审核模式
  if grep -q 'auti-mode' /proc/cmdline; then
        do_auti_mode
  fi

  case ${LANGUAGES} in
        'zh_CN')
                echo "系统出厂备份完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "System factory backup completed" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "མ་ལག་ནས་ཐོན་པའི་备份ལེགས་གྲུབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "系統出廠備份完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠱᠢᠰᠲ᠋ᠧᠮ ᠦ᠋ᠨ ᠦᠢᠯᠡᠳᠪᠦᠷᠢ ᠡᠴᠡ ᠭᠠᠷᠭᠠᠬᠤ ᠪᠡᠯᠡᠳᠭᠡᠯ ᠠᠵᠢᠯ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "系统出厂备份完成" >>${LOG_PROGRESS_FILE}
                ;;
  esac

}

do_install_success() {
  if [[ "${is_990_9a0}" == "true" ]]; then
    mkdir -p /target/boot/efi
    touch /target/boot/efi/INSTALL_SUCCESS
  fi
}

handle_factory_backup() {
  factory_backup=$(get_value factory-backup)
  #if [[ -f /target/etc/.bootinfo ]]; then
    if grep -q 'factory-backup' /proc/cmdline; then
      do_factory_backup
    elif [[ $factory_backup -eq 1 ]]; then
      do_factory_backup
    fi
  #fi
}

do_finished() {
  echo 'finished'
  touch /tmp/z-finished-install
}

do_os_prober() {
  dmsetup ls |while read line
  do
        if [ "$line" = "No devices found" ];then
                break
        fi
        PART=`echo "$line"|awk '{print $1}'`
        dmsetup remove $PART || true
  done
}

do_select_disk() {
  if grep -q automatic /proc/cmdline; then
        devpath=$(get_value devpath)
        if [[ -z $devpath ]]; then
                 auto_disk >>${LOG_FILE}
        elif [[ $devpath == "min" ]]; then
                 auto_disk_min >>${LOG_FILE}
        elif [[ $devpath == "speed" ]]; then
                 auto_disk_speed >>${LOG_FILE}
        elif [[ $devpath == "M.2_0" ]]; then
                 auto_disk_slot $devpath >>${LOG_FILE}
        elif [[ $devpath == "M.2_1" ]]; then
                 auto_disk_slot $devpath >>${LOG_FILE}
        else
                 get_disk  >>${LOG_FILE}
        fi
  elif grep -qE 'ple-mode|test-mode' /proc/cmdline; then
        devpath=$(get_value devpath)
        if [[ -z $devpath ]]; then
                 auto_disk >>${LOG_FILE}
        elif [[ $devpath == "min" ]]; then
                 auto_disk_min >>${LOG_FILE}
        elif [[ $devpath == "speed" ]]; then
                 auto_disk_speed >>${LOG_FILE}
        elif [[ $devpath == "M.2_0" ]]; then
                 auto_disk_slot $devpath >>${LOG_FILE}
        elif [[ $devpath == "M.2_1" ]]; then
                 auto_disk_slot $devpath >>${LOG_FILE}
        else
                 get_disk  >>${LOG_FILE}
        fi
  fi

  data_device=$(get_value data-device)
  if [[ ${data_device} == "auto" ]]; then
        automatic=$(get_value automatic-installation)
        if [[ "$automatic" == "0" ]]; then
                echo "GUI custom installation"
        else
                get_data_disk >>${LOG_FILE}
        fi
  fi
}

do_apps_third() {
  new_sysroot_dir=$(get_value new-root)
  if [[ -d /cdrom/apps-third ]] && [[ "${is_ghost}" != "true" ]]; then
        if [[ -d $new_sysroot_dir/etc/apt/sources.list.d ]]; then
                echo "$new_sysroot_dir/etc/apt/sources.list.d is exit"
        else
                mkdir -p $new_sysroot_dir/etc/apt/sources.list.d
        fi
        echo "deb [trusted=yes] file:///opt/apps-third/ ./" >$new_sysroot_dir/etc/apt/sources.list.d/kylin-apps-third.list
        if [[ -f $new_sysroot_dir/etc/apt/sources.list ]]; then
                mv $new_sysroot_dir/etc/apt/sources.list $new_sysroot_dir/etc/apt/sources.list.bak
        fi
        chroot $new_sysroot_dir apt-get update --allow-insecure-repositories -y || true
        if [[ -f $new_sysroot_dir/etc/apt/sources.list.bak ]]; then
                mv $new_sysroot_dir/etc/apt/sources.list.bak $new_sysroot_dir/etc/apt/sources.list
        fi
  fi
}

rm_deploy_systemroot_usr_config() {
  cat "/usr/share/kylin-os-installer/scripts/ostree_system_usr_etc_config" | while read filename
  do
    echo "$filename"
    realPathFilename="$1$filename"
    [ -f $realPathFilename ] && rm $realPathFilename
  done

}

get_ostree_release_file_info() {
	while read os_release_file_line
	do
		if [ "$os_release_file_line" = "[info]" ]; then
			sign_position=0
			continue
		fi

		if [ $sign_position = 0 ] && [ "$os_release_file_line" != "[info]" ]; then
			info_version_string=$os_release_file_line
			sign_position=1
			continue
		fi

		if [ "$os_release_file_line" != "[metadata]" ]; then
			mate_data_add_string="$mate_data_add_string --add-metadata-string=$os_release_file_line"
		fi

	done < "/etc/ostree-release"

	info_version_string=${info_version_string#*version=}
	os_version_name=$(echo "$info_version_string" | awk -F ":" '{print $1}')
	echo "$os_version_name" >> ${LOG_FILE} 2>&1
	echo "$info_version_string" >> ${LOG_FILE} 2>&1
	echo "$mate_data_add_string" >> ${LOG_FILE} 2>&1
}


# 解决os-prober问题
do_os_prober >>${LOG_FILE} 2>&1

#安装自动选择磁盘安装方案
do_select_disk >>${LOG_FILE} 2>&1

# 运行 prepare 阶段脚本
do_prepare >>${LOG_FILE} 2>&1

# 执行 target inchroot 脚本
do_run_target


# 运行 after 阶段脚本
do_after >>${LOG_FILE} 2>&1

factory_backup=$(get_value factory-backup)
if grep -q 'factory-backup' /proc/cmdline; then
	echo "installer-progress=92" >${INSTALLER_PROGRESS}
	echo "installer-progress-next=96" >>${INSTALLER_PROGRESS}
elif [[ $factory_backup -eq 1 ]]; then
	echo "installer-progress=92" >${INSTALLER_PROGRESS}
	echo "installer-progress-next=96" >>${INSTALLER_PROGRESS}
else
	echo "installer-progress=92" >${INSTALLER_PROGRESS}
	echo "installer-progress-next=99" >>${INSTALLER_PROGRESS}
fi

# 执行 postactions 脚本
do_cdrom_post_actions

# 执行离线包升级
do_update_installer >>${LOG_FILE} 2>&1

#配置apps-third源
do_apps_third >>${LOG_FILE} 2>&1

# 拷贝overlay 数据至备份还原点中
do_copy_overlay_data_backup_deploy

# 修改权限
do_chown_sysroot_permissions

# 拷贝安装日志
do_copy_log

do_install_success

# 处理出厂备份
handle_factory_backup >>${LOG_FILE} 2>&1

case ${LANGUAGES} in
        'zh_CN')
                echo "系统安装完成前的准备..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "Preparation before system installation is completed..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "མ་ལག་སྒྲག་སྦྱར་ལེགས་གྲུབ་སྔོན་གྱི་གྲ་སྒྲག།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "系統安裝完成前的準備..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo " ᠰᠢᠰᠲ᠋ᠡᠮ ᠤᠨ ᠰᠠᠭᠤᠯᠭᠠᠯᠲᠠ ᠳᠠᠭᠤᠰᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ ᠪᠡᠯᠡᠳᠬᠡᠯ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "系统安装完成前的准备..." >>${LOG_PROGRESS_FILE}
                ;;
esac

#创建安装成功标准
new_sysroot_dir=$(get_value new-root)
touch $new_sysroot_dir/var/log/installer/INSTALL_SUCCESS

if [[ -f $new_sysroot_dir/etc/fstab ]]; then
        cp -a $new_sysroot_dir/etc/fstab /target/etc
fi
if [[ -f $new_sysroot_dir/etc/lsb-release ]]; then
        cp -a $new_sysroot_dir/etc/lsb-release /target/etc
fi

#审核模式设置成运维模式
if grep -q 'auti-mode' /proc/cmdline; then
        new_sysroot_dir=$(get_value new-root)
        deploy_hashvalue_root=$(get_value deploy_hashvalue_root)
        system_ovl_conf=${new_sysroot_dir}/sysroot/ostree/deploy/kylin/deploy/${deploy_hashvalue_root}/sysroot/ostree/pkgs/system-ovl.conf

	if [[ -f ${new_sysroot_dir}/sysroot/ostree/pkgs/ostree-version.info  ]]; then
		kpb_hashvalue_root=`cat ${new_sysroot_dir}/sysroot/ostree/pkgs/ostree-version.info | awk -F " " '{print $2}'` || true
		if [[ -f ${new_sysroot_dir}/sysroot/ostree/deploy/kylin/deploy/${kpb_hashvalue_root}/sysroot/ostree/pkgs/system-ovl.conf  ]]; then
        		system_ovl_conf=${new_sysroot_dir}/sysroot/ostree/deploy/kylin/deploy/${kpb_hashvalue_root}/sysroot/ostree/pkgs/system-ovl.conf
		fi
	fi

        echo "${system_ovl_conf}" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
        echo "start set Mode=maintain" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
        if [[ -f ${system_ovl_conf} ]]; then
                cp -a ${system_ovl_conf} $new_sysroot_dir/var/log/installer/system-ovl-old.conf
                echo "${system_ovl_conf} is exit" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
                if ! grep -q '[General]' ${system_ovl_conf}; then
                        echo "[General]" >> ${system_ovl_conf}
                fi
        else
                echo "[General]" >> ${system_ovl_conf}
        fi
        if grep -q 'Mode=normal' ${system_ovl_conf}; then
                echo "set Mode=maintain11" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
                sed -i  's/'"Mode=normal"'/'"Mode=maintain"'/' ${system_ovl_conf}
        else
                echo "set Mode=maintain22" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
                echo "Mode=maintain" >>${system_ovl_conf}
        fi

        if grep -q 'NextMode=normal' ${system_ovl_conf}; then
                echo "set Mode=maintain33" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
                sed -i  's/'"NextMode=normal"'/'"NextMode=maintain"'/' ${system_ovl_conf}
        else
                echo "set Mode=maintain44" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
                echo "NextMode=maintain" >>${system_ovl_conf}
        fi

        if [[ -f /target/ostree/pkgs/mm.lock ]]; then
                echo "/target/ostree/pkgs/mm.lock is exit" >>$new_sysroot_dir/var/log/installer/kylin-os-installer-detail.log
        else
                touch /target/ostree/pkgs/mm.lock
        fi
fi

echo "installer-progress=99" >${INSTALLER_PROGRESS}
echo "installer-progress-next=100" >>${INSTALLER_PROGRESS}
# 卸载文件系统
umount_all  >>${LOG_FILE} 2>&1

do_finished

case ${LANGUAGES} in
        'zh_CN')
                echo "系统安装完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "System installation completed" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "མ་ལག་སྒྲག་སྦྱར་ལེགས་གྲུབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "系統安裝完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo " ᠰᠢᠰᠲ᠋ᠡᠮ ᠤᠨ ᠰᠠᠭᠤᠯᠭᠠᠯᠲᠠ ᠪᠡᠶᠡᠯᠡᠭᠳᠡᠬᠦ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "系统安装完成" >>${LOG_PROGRESS_FILE}
                ;;
esac

