#!/bin/bash 
osname=$1
csum=$2
deployserial=$3
bootcsum=$4
deploy_path=/ostree/deploy/${osname}/deploy/${csum}.${deployserial}
 
# 定义目标目录  
TARGET_DIR="${deploy_path}/usr/lib/modules"  
#TARGET_DIR="/usr/lib/modules"  ##在chroot环境内，环境外路径为：/ostree/deploy/openkylin/deploy/${commit_id}.0/usr/lib/modules

# 遍历目录下的所有子目录  
for ker_version in "$TARGET_DIR"/*; do  
	echo $ker_version
    if [[ -d "$ker_version" ]]; then  
        # 进入子目录  
        cd "$ker_version" || continue  
  		echo "dir="$ker_version
        # 获取子目录名称  
        subdir_name=$(basename "$ker_version")  
		echo "sub="${subdir_name}

        ## 进入deploy的环境并创建img文件
        chroot $deploy_path mkinitramfs -o /etc/initramfs-${subdir_name}.img ${subdir_name} -v
        initramfs_file=${deploy_path}/etc/initramfs-${subdir_name}.img
        if [ -s $initramfs_file ] && [ -e $initramfs_file ]
        then 
            cp ${deploy_path}/etc/initramfs-${subdir_name}.img  /boot/ostree/${osname}-${bootcsum}/
        fi
        echo "------------------------------"  
          
    fi  
done
