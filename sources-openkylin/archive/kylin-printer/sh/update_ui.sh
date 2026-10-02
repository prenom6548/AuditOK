#!/bin/bash

set -e
shopt -s expand_aliases

project_path=$(dirname `realpath $0`)
source ${project_path}/common.sh

cd $src_dir

file_path_list=`find . -name *.ui`

if [ ! -d ${src_dir}/include ]; then
    ${MKDIR} ${src_dir}/include
fi

for file_path in ${file_path_list[@]}; do
    # 使用basename获取文件名（包括扩展名）
    base_name=$(basename "$file_path")
    # 剥离扩展名
    file_name="${base_name%.*}"

    uic -o $src_dir/include/ui_${file_name}.h ${file_path}
done
