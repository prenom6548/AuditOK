#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
# 初始化一个数组用于存储 id/version/module/channel 信息
install_info=()
architecture=$(arch)
# 使用 kaiming search "" 检索信息并提取字段
sudo kaiming search --arch=$architecture "" | awk 'NR > 2' | while IFS= read -r line; do
    
    # 提取第一个字段作为 id
    id=$(echo "$line" | awk '{print $1}')
    if [[ -z "$id" ]]; then
        continue
    fi
    # 提取剩余部分，去掉 id 后的所有内容
    remaining=$(echo "$line" | sed -E "s/^$id //")

    # 初始化变量
    name=""
    version=""
    channel=""
    model=""
    install_info=""
    i=0
    # 将 remaining 按空格拆分成数组
    read -r -a temp_fields <<< "$remaining"
    while [[ -n "${temp_fields[i]}" ]]; do
        if [[ "${temp_fields[i]:0:1}" =~ [0-9] ]]; then
	    #echo "${temp_fields[i]:0:1}"
        version="${temp_fields[i]}"
	    channel="${temp_fields[i+1]}"
	    module="${temp_fields[i+4]}"
	    break
        else
            ((i++))
        fi
    done
#    echo "$name,$version,$channel,$model"
    if [[ -n "$id" ]]; then
        install_info=("$id/$version/$module/$channel")
        echo "$install_info"
	    sudo kaiming install  -y "$install_info"
	    found_dir=$(find /opt/kaiming/layers/$channel -type d -name "$id" 2>/dev/null)
	    if [ -n "$found_dir" ]; then
   	        echo "$found_dir"
	        target_dir="$found_dir/$module/$version"
	  
	        if [ -d "$target_dir" ]; then
                echo "安装成功:$install_info"
	        else 
	            echo "安装失败:$install_info"
                exit 1
	        fi
        else
            echo "安装失败:$install_info"
            exit 1
        fi

    fi
    
    
done
