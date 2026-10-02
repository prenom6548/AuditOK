#!/bin/bash

set -e

get_is_efi() {
    if type archdetect >/dev/null 2>&1; then
        archdetect=$(archdetect)
    else
        archdetect=unknown/generic
    fi
    arch=${archdetect%/*}
    sub=${archdetect#*/}
    ret=true
    case "$arch" in
        alpha|sw64)
            ret=true;;
        amd64|i386)
            case "$sub" in
                mac|efi)
                ret=true;;
                *)
                ret=false;;
            esac;;
        arm|armhf|armel)
            ret=false;;
        arm64)
            ret=true;;
        mips64el)
            case "$sub" in
                efi)
                ret=true;;
                *)
                ret=false;;
            esac;;
        mips|mipsel)
            ret=false;;
        *)
            ret=true;;
    esac

    # 华为 990 设置为 efi
    if egrep -qi 'kirin.?9[09]0' /proc/cpuinfo; then
        ret=true
    fi
    if egrep -qi 'PANGU.M900' /proc/cpuinfo; then
        ret=true
    fi

    echo $ret
}

do_backup_size() {
	backup_size=$(( real_size / 9 ))
	if [ $backup_size -lt 16384 ]; then
    		backup_size=16384
	elif [ $backup_size -gt 102400 ]; then
    		backup_size=102400
	fi
}

disk=$1

size=$(fdisk -s "${disk}")
real_size=$(( size / 1024 ))

### efi
efi=512
if [[ "${is_efi}" == "false" ]]; then
	efi=0
fi

### boot
boot=2048

### backup
backup_size=0
systemd-detect-virt || do_backup_size

ram=$(cat < /proc/meminfo  | grep ^MemTotal: | awk '{print $2}')

### swap
swap=$(( ram * 12 / 1024 / 10 ))
if [ $swap -gt 64000 ]; then
    swap=64000
fi
swapfile=0
if grep -q kyswapfile /proc/cmdline; then
    swapfile=${swap}
    swap=0
fi

free_size=$(( real_size - efi - boot - backup_size - swap ))

### root
root_size=$(( free_size * 6 / 10 ))
if [ $root_size -lt 40960 ]; then
    root_size=40960
    if [ $free_size -lt 0 ]; then
        exit 1
    fi
elif [ $root_size -gt 102400 ]; then
    root_size=102400
    if [ $free_size -lt 204800 ]; then
    	root_size=$(( free_size * 5 / 10 ))
    fi
fi
root_size=$((root_size + swapfile))

#system_min_size=$(( real_size - free_size + 16384 + swapfile ))
#system_max_size=$(( real_size * 8 / 10 ))

data_min_size=$(( real_size * 2 / 10 ))
data_max_size=$(( free_size - 40960 - swapfile ))

#系统和数据分区大小
data_default_size=$(( free_size - root_size ))
system_default_size=$(( real_size - data_default_size ))


echo "swap_default_size: $swap"
echo "backup_default_size: $backup_size"
echo "root_default_size: $root_size"

echo "data_default_size: $data_default_size"
echo "system_default_size: $system_default_size"
echo "data_min_size: $data_min_size"
echo "data_max_size: $data_max_size"

echo "data partition and system partition " >/var/log/installer/data-system-size.log || true
echo "data-default-size=${data_default_size}" >>/var/log/installer/data-system-size.log
echo "system-default-size=${system_default_size}" >>/var/log/installer/data-system-size.log
echo "data-min-size=${data_min_size}" >>/var/log/installer/data-system-size.log
echo "data-max-size=${data_max_size}" >>/var/log/installer/data-system-size.log

echo "swap-default-size=${swap}" >>/var/log/installer/data-system-size.log
echo "backup-default-size=${backup_size}" >>/var/log/installer/data-system-size.log
echo "root-default-size=${root_size}" >>/var/log/installer/data-system-size.log

