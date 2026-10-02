#!/bin/bash

set -e

boot_size=0
root_size=0
backup_size=0
data_size=0
swap_size=0

data_device=$(get_value data-device)

disk=$(get_value devpath)
size=$(fdisk -s "${disk}")
real_size=$(( size / 1024 ))

kos_end=$(get_value kos-end)
if [ -z ${kos_end} ]; then
        free_size=${real_size}
else
        if [[ $kos_end == "0" ]]; then
        	free_size=${real_size}
        else
                free_size=$(( real_size - kos_end ))
        fi
fi

custom_partitions=$(get_value custom-partitions | sed 's/\"//g')
one_list=${custom_partitions//;/ }
two_list=
for p in ${one_list[@]}; do
        two_list="${two_list} $(echo $p | cut -f1)"
done
three_list=$(echo ${two_list} | tr ' ' '\n')

for i in ${three_list}; do
        custom_partition=$(get_value "custom-$i" | sed 's/\"//g')
        custom_partition=${custom_partition//;/|}
        
	size=
        size=$(echo ${custom_partition#*size=})
        size=$(echo ${size%%|*})

        if [[ $i == "efi" ]]; then
                continue
        fi

        if [[ $i == "boot" ]]; then
                if [[ $size == "default" ]]; then
                        boot_size=2048
		else
                        boot_size=$size
                fi
		free_size=$(( free_size - boot_size ))
                continue
        fi

        if [[ $i == "root" ]]; then
                continue
        fi

	if [[ $i == "backup" ]]; then
                if [[ $size == "default" ]]; then
                        backup_size=$(( real_size / 9 ))
                        if [ $backup_size -lt 16384 ]; then
                                backup_size=16384
                        elif [ $backup_size -gt 102400 ]; then
                                backup_size=102400
                        fi

                        if [[ "${virtual_machine}" == "true" ]]; then
                                backup_size=0
                        fi

                        if [[ -n ${data_device} ]]; then
                                backup_size=default
                        else
                                free_size=$(( free_size - backup_size ))
                        fi

                        echo "backup_size=${backup_size}"
                else
                        backup_size=$size
                        free_size=$(( free_size - backup_size ))
                fi
                continue
        fi


        if [[ $i == "data" ]]; then
                continue
        fi

        if [[ $i == "swap" ]]; then
                if [[ $size == "default" ]]; then
                	ram=$(cat < /proc/meminfo  | grep ^MemTotal: | awk '{print $2}')
			swap_size=$(( ram * 12 / 1024 / 10 ))
			if [ ${swap_size} -gt 64000 ]; then
    				swap_size=64000
			fi
			swapfile=0
			if [[ "${is_swapfile}" == "true" ]]; then
    				swapfile=${swap}
    				swap_size=0
			fi
		else
			swap_size=$size
		fi
		free_size=$(( free_size - swap_size ))
		continue
        fi

	if [[ $size == "free-size" ]]; then
		continue
        else
		free_size=$(( free_size - size ))
        fi
done

if [[ -n ${data_device} ]]; then
		
	data_size=0
        if [[ ${backup_size} == "default" ]]; then
                for i in ${three_list}; do
                        custom_partition=$(get_value "custom-$i" | sed 's/\"//g')
                        custom_partition=${custom_partition//;/|}

                        size=
                        size=$(echo ${custom_partition#*size=})
                        size=$(echo ${size%%|*})

                        if [[ $i == "root" ]]; then
                                if [[ $size == "default" ]]; then
                                        root_size=$(( free_size * 5 / 10 ))
                                        backup_size=$(( free_size - root_size ))
					if [ $backup_size -gt 102400 ]; then
                                                backup_size=102400
                                                root_size=$(( free_size - backup_size ))
                                        fi
                                else
                                        root_size=$size
                                        backup_size=$(( free_size - root_size ))
                                fi
                                continue
                        fi
                        continue
                done
        else
                for i in ${three_list}; do
                        custom_partition=$(get_value "custom-$i" | sed 's/\"//g')
                        custom_partition=${custom_partition//;/|}

                        size=
                        size=$(echo ${custom_partition#*size=})
                        size=$(echo ${size%%|*})

                        if [[ $i == "root" ]]; then
                                if [[ $size == "default" ]]; then
                                        root_size=${free_size}
                                else
                                        root_size=$size
                                        free_size=$(( free_size - root_size ))
                                        if [ $free_size -lt 0 ]; then
                                                exit 1
                                        fi
                                fi
                                continue
                        fi
                        continue
                done

        fi

else
	for i in ${three_list}; do
        	custom_partition=$(get_value "custom-$i" | sed 's/\"//g')
        	custom_partition=${custom_partition//;/|}

        	size=
        	size=$(echo ${custom_partition#*size=})
        	size=$(echo ${size%%|*})

        	if [[ $i == "root" ]]; then
                	if [[ $size == "default" ]]; then
				root_size=$(( free_size * 6 / 10 ))
				if [ $root_size -lt 16384 ]; then
    					root_size=16384
				elif [ $root_size -gt 102400 ]; then
    					root_size=102400
					if [ $free_size -lt 204800 ]; then
                                                root_size=$(( free_size * 5 / 10 ))
                                        fi
				else
                                        root_size=$(( free_size * 5 / 10 ))
				fi
				root_size=$((root_size + swapfile))
			else
				root_size=$size
			fi
			free_size=$(( free_size - root_size ))
			if [ $free_size -lt 0 ]; then
        			exit 1
    			fi
        		continue
        	fi
                continue
	done

	for i in ${three_list}; do
                custom_partition=$(get_value "custom-$i" | sed 's/\"//g')
                custom_partition=${custom_partition//;/|}

                size=
                size=$(echo ${custom_partition#*size=})
                size=$(echo ${size%%|*})

        	if [[ $i == "data" ]]; then
                	if [[ $size == "default" ]]; then
				data_size=${free_size}
                	else
                        	data_size=$size
                	fi
			if [ $data_size -lt 0 ]; then
   				exit 1
			fi
                	continue
        	fi
        	continue
        done
fi

echo "boot-size=${boot_size}" >>/tmp/ky-installer.cfg
echo "root-size=${root_size}" >>/tmp/ky-installer.cfg
echo "backup-size=${backup_size}" >>/tmp/ky-installer.cfg
echo "data-size=${data_size}" >>/tmp/ky-installer.cfg
echo "swap-size=${swap_size}" >>/tmp/ky-installer.cfg

