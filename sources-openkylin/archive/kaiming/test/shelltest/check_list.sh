#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
architecture=$(arch)
base_dir=$(find /opt/kaiming/layers/ -type d -name "$architecture" 2>/dev/null)

# 初始化数组
third_level_dirs=()

# 使用 find 查找第三层子目录，并将其名称存入数组
while IFS= read -r dir; do
    # 去掉前缀路径，只保留三层目录
    relative_path=${dir#"$base_dir"}
    three_levels=$(echo "$relative_path" | awk -F'/' '{print $3"/"$4"/"$5}')
    third_level_dirs+=("$three_levels")
done < <(find "$base_dir" -mindepth 5 -maxdepth 5 -type d)

# 输出数组内容
IFS=$'\n' sorted_refs=($(printf "%s\n" "${third_level_dirs[@]}" | sort -u))
unset IFS


result=$(kaiming list | awk 'NR > 1')
# echo $result
install_info=()
while IFS= read -r line; do
    # 提取第一个字段作为 id
    id=$(echo "$line" | awk '{print $1}')
    if [[ -z "$id" ]]; then
        continue
    fi

    # 提取剩余部分，去掉 id 后的所有内容
    remaining=$(echo "$line" | sed -E "s/^$id //")
    # 初始化变量
    version=""
    channel=""
    module=""
    i=0

    # 将 remaining 按空格拆分成数组
    read -r -a temp_fields <<< "$remaining"
    while [[ -n "${temp_fields[i]}" ]]; do
        if [[ "${temp_fields[i]:0:1}" =~ [0-9] ]]; then
            version="${temp_fields[i]}"
            channel="${temp_fields[i+1]}"
            module="${temp_fields[i+5]}"
            break
        else
            ((i++))
        fi
    done

    if [[ -n "$id" && -n "$module" && -n "$version" ]]; then
        install_info+=("$id/$module/$version")
    fi
done <<< "$result"

IFS=$'\n' sorted=($(printf "%s\n" "${install_info[@]}" | sort -u))
unset IFS

for ref in "${sorted_refs[@]}"; do
    found=false
    for item in "${sorted[@]}"; do
        if [[ "$item" == "$ref" ]]; then
            echo "Found:$ref"
            found=true
            break
        fi
    done
    if [[ "$found" == false ]]; then
        echo "list缺少:$ref"
        exit 1
    fi
done

