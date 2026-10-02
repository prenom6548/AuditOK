#!/bin/bash

set -e

# /boot     1024M
# /         6
# /backup   free/9
# /data     4
# swap      ram*1.2

if [[ "${is_efi}" == "true" ]]; then
    return
fi

disk=$(get_value devpath)
size=$(fdisk -s "${disk}")
real_size=$(( size / 1024 ))

disk_custom=$(get_value disk-custom)

if [[ "${disk_custom}" == "true" ]]; then


#    efi=$(get_value disk-efi)
    boot=$(get_value disk-boot)
    backup_size=$(get_value disk-backup)
    swap=$(get_value disk-swap)
    root_size=$(get_value disk-root)
    data_size=$(get_value disk-data)


#    efi=$efi
    boot=$boot
    backup_size=$backup_size
    swap=$swap
    root_size=$root_size
    data_size=$data_size
 
    disk_size=$(( real_size  - boot - backup_size - swap - root_size - data_size ))

    if [ $disk_size -lt 0 ]; then
        echo " the size of disk_custom is false " || exit 1
    fi


#    echo "efi: $efi"
    echo "boot: $boot"
    echo "root_size: $root_size"
    echo "backup_size: $backup_size"
    echo "data_size: $data_size"
    echo "swap: $swap"
#    echo "swapfile: $swapfile"
    return 0
fi



### boot
boot=2048


### backup
backup_size=$(( real_size / 9 ))
if [ $backup_size -lt 16384 ]; then
    backup_size=16384
elif [ $backup_size -gt 102400 ]; then
    backup_size=102400
fi
if [[ "${virtual_machine}" == "true" ]]; then
    backup_size=0
fi


ram=$(cat < /proc/meminfo  | grep ^MemTotal: | awk '{print $2}')


### swap
swap=$(( ram * 12 / 1024 / 10 ))
if [ $swap -gt 64000 ]; then
    swap=64000
fi
swapfile=0
if [[ "${is_swapfile}" == "true" ]]; then
    swapfile=${swap}
    swap=0
fi
free_size=$(( real_size - boot - backup_size - swap ))


### root
root_size=$(( free_size * 6 / 10 ))
if [ $root_size -lt 16384 ]; then
    root_size=16384
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

### data
data_size=$(( free_size - root_size ))
if [ $data_size -lt 0 ]; then
    exit 1
fi

data_customize_size=0
data_customize_size=$(get_value data-customize-size) || data_customize_size=0
if [ $data_customize_size -gt 0 ]; then
        data_size=${data_customize_size}
        root_size=$(( free_size - data_size ))
fi

data_device=$(get_value data-device)
if [[ -n ${data_device} ]]; then
        if [[ "${data_device}" == "false" ]]; then
                echo "data-customize-size=false"
        else
        	free_size=$(( real_size - boot - swap ))
        	root_size=$(( free_size * 5 / 10 ))
        	backup_size=$(( free_size - root_size ))
		if [ $backup_size -gt 102400 ]; then
                	backup_size=102400
                	root_size=$(( free_size - backup_size ))
        	fi
        	data_size=0
        fi
fi

echo "boot: $boot"
echo "root_size: $root_size"
echo "backup_size: $backup_size"
echo "data_size: $data_size"
echo "swap: $swap"
echo "swapfile: $swapfile"
