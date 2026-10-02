#!/bin/bash

set -e


# variables
projet_name='kylin-printer'
project_path=$(dirname `realpath $0`)
src_dir=${project_path}/..
translations_dir=$src_dir/translations
po_dir=$src_dir/po
shell_dir=$src_dir/sh
locales=('en'
    'zh_CN'
    'bo_CN'
    'kk_KZ'
    'kk'
    'ky_KG'
    'ky'
    'ug_CN'
    'ug'
    'ru'
    'mn'
    'vi'
    'ms'
    'th'
    'zh_HK'
    'zh_Hant'
    )

declare -A ts_files_head
ts_files_head[${translations_dir}/${projet_name}_]="\
${src_dir}/backend \
${src_dir}/common \
${src_dir}/printer_manager \
${src_dir}/device_manager \
${src_dir}/ui \
${src_dir}/util"
ts_files_head[${translations_dir}/${projet_name}_dict_]="\
${src_dir}/dictionary/"


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
