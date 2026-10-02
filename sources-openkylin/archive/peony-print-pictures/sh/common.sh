#!/bin/bash

set -e


# variables
projet_name='peony-print-pictures'
project_path=$(dirname `realpath $0`)
src_dir=${project_path}/..
translations_dir=$src_dir/translations
po_dir=$src_dir/po
shell_dir=$src_dir/sh
function get_locales()
{
    list=$(find "$translations_dir" -maxdepth 1 -type f)
    for file_path in ${list[@]}; do
        filename=$(basename "$file_path")
        locale=${filename#*_}  # 去掉第一个 _ 之前的部分
        locale=${locale%.*}    # 去掉 . 之后的部分
        echo $locale
    done
}

locales=$(get_locales)

declare -A ts_files_head
ts_files_head[${translations_dir}/${projet_name}_]="\
${src_dir}/kylin-photo-printer \
${src_dir}/peony-print-pictures"


# alias
RM='rm -rf'
MKDIR='mkdir'
CD='cd'
LUPDATE='lupdate -silent'
LRELEASE='lrelease -silent'
case `uname` in
    Linux)
        SED_I='sed -i '
    ;;
    Darwin)
        SED_I='sed -i "" '
    ;;
    *)
    ;;
esac


# fun
function clear_po()
{
    ${CD} ${src_dir}
    ${RM} ${po_dir}/*.ts \
          ${po_dir}/*.po \
          ${po_dir}/*.pot \
          ${po_dir}/.intltool-merge-cache
    find . -name ".cvsignore" | xargs ${RM}
    find . -name "Makefile.am" | xargs ${RM}
    ${CD} $OLDPWD
}

function ts_files_check()
{
    all_checked=0
    for locale in ${locales[@]}; do
        for ts_file in ${!ts_files_head[@]}; do
            if [ ! -f ${ts_file}${locale}.ts ]; then
                echo "File ${ts_file}${locale}.ts didn't exist!"
                all_checked=1
            fi
        done
    done
    return ${all_checked}
}
