#!/bin/bash

set -e

. /tmp/utils

#timedatectl set-ntp true
mkdir -p /var/log/installer

do_inchroot() {
  if [[ -d /cdrom/hooks/in_chroot ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行配置前in_chroot定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "In_chroot custom script before executing configuration..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་བཀོད་སྒྲིག་གོང་in_chrootངེས་སྲོལ་འཁྲབ་དེབ།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行配寘前in_chroot定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠲᠣᠬᠢᠷᠠᠭᠤᠯᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ in_chroot ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行配置前in_chroot定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /cdrom/hooks/in_chroot || do_error hooks-inchroot
    case ${LANGUAGES} in
        'zh_CN')
                echo "配置前in_chroot定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "In_chroot custom script execution completed before configuration" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "བཀོད་སྒྲིག་གོང་in_chrootངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲུབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "配寘前in_chroot定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠲᠣᠬᠢᠷᠠᠭᠤᠯᠬᠤ ᠠ᠋ᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ in_chrootᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠬᠦ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "配置前in_chroot定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
  fi
  
  if [[ -d /tmp/in_chroot ]]; then
    case ${LANGUAGES} in
        'zh_CN')
                echo "执行配置前in_chroot定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "In_chroot custom script before executing configuration..." >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "ལག་བསྟར་བཀོད་སྒྲིག་གོང་in_chrootངེས་སྲོལ་འཁྲབ་དེབ།..." >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "執行配寘前in_chroot定制腳本..." >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠲᠣᠬᠢᠷᠠᠭᠤᠯᠬᠤ ᠠᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ in_chroot ᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ..." >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "执行配置前in_chroot定制脚本..." >>${LOG_PROGRESS_FILE}
                ;;
    esac
    run_scripts_hooks /tmp/in_chroot || do_error hooks-inchroot
    case ${LANGUAGES} in
        'zh_CN')
                echo "配置前in_chroot定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'en_US')
                echo "In_chroot custom script execution completed before configuration" >>${LOG_PROGRESS_FILE}
                ;;
        'bo_CN')
                echo "བཀོད་སྒྲིག་གོང་in_chrootངེས་སྲོལ་འཁྲབ་དེབ་ལག་བསྟར་ལེགས་སྒྲུབ།" >>${LOG_PROGRESS_FILE}
                ;;
        'zh_HK')
                echo "配寘前in_chroot定制腳本執行完成" >>${LOG_PROGRESS_FILE}
                ;;
        'mn_MN')
                echo "ᠲᠣᠬᠢᠷᠠᠭᠤᠯᠬᠤ ᠠ᠋ᠴᠠ ᠡᠮᠦᠨᠡᠬᠢ in_chrootᠦᠢᠯᠡᠳᠬᠦ ᠦᠯᠢᠭᠡᠷ ᠳᠡᠪᠲᠡᠷ ᠢ᠋ ᠬᠡᠷᠡᠭᠵᠢᠭᠦᠯᠬᠦ" >>${LOG_PROGRESS_FILE}
                ;;
              *)
                echo "配置前in_chroot定制脚本执行完成" >>${LOG_PROGRESS_FILE}
                ;;
    esac
    rm -rf /tmp/in_chroot
  fi
  # 运行 "${IN_TARGET}"/in_chroot/ 脚本
  run_scripts_dir "${IN_TARGET}"/in_chroot
}

do_inchroot

msg "k.sh 运行结束"
