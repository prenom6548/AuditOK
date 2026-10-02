#
# Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
#
# kaiming is licensed under the GPL v2.0+.
#
# See the LICENSE file for more details.
#

#!/bin/bash
if [ $# -eq 0 ]; then
    echo "请提供应用类型作为参数"
    exit 1
fi
kind=$1

base_dirs=("/opt/kaiming/layers/stable/" "$HOME/.local/share/kaiming/layers/stable/")

# 初始化数组
third_level_dirs=()

# 使用 find 查找子目录，并将其名称存入数组
for base_dir in "${base_dirs[@]}"; do
    if [ ! -d "$base_dir" ]; then
        echo "警告: 路径不存在 -> $base_dir"
        continue
    fi
    while IFS= read -r dir; do
        # 去掉前缀路径，只保留三层目录
        relative_path=${dir#"$base_dir"}
        # 修改 awk 命令来检查第2个字段是否等于 $kind
        three_levels=$(echo "$relative_path" | awk -F'/' -v kind="$kind" '$2==kind {print $3"/"$4"/"$5}')
        # 只有当 three_levels 不为空时才添加到数组
        if [ ! -z "$three_levels" ]; then
            third_level_dirs+=("$three_levels")
        fi
    done < <(find "$base_dir" -mindepth 5 -maxdepth 5 -type d)
done
# 输出数组内容
IFS=$'\n' sorted_refs=($(printf "%s\n" "${third_level_dirs[@]}" | sort -u))
unset IFS

result=$(kaiming list --kind=$kind | awk 'NR > 1')
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

for ((i=0; i < ${#sorted_refs[@]}; i++)); do
    # echo "$i:${sorted[i]},          $i:${sorted_refs[i]}"
    if [[ "${sorted[0]}" == "${sorted_refs[0]}" ]]; then
        echo "Found:${sorted_refs[i]}"
    else
	    echo "list缺少:${sorted_refs[i]}"
	    exit  1
    fi
done
