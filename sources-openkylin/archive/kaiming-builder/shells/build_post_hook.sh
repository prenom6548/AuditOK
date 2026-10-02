#!/bin/bash

# 此脚本需要在构建完成后在容器构建环境中执行，旨在构建完成后自动做一些通用性操作，以简化开发者配置

function compile_gschema() {

    # 多个来源的模式文件
    local SCHEMA_DIRS=(
        "/share/glib-2.0/schemas"
        "/local/share/glib-2.0/schemas"
        "/usr/share/glib-2.0/schemas"
        "/usr/local/share/glib-2.0/schemas"
    )
    
    # 创建临时目录
    local schema_dir="${KAIMING_BUILDER_BUILDDIR}/tmp_schema_dir"
    rm -rf ${schema_dir}
    mkdir -p ${schema_dir}

    # 将应用自身的模式文件拷贝到临时目录
    local app_schema_dir=
    for dir in "${SCHEMA_DIRS[@]}"; do
        if [ -d "${APP_DIRECTORY}${dir}" ]; then
            cp -n ${APP_DIRECTORY}${dir}/* ${schema_dir}/ 2>/dev/null || true
            app_schema_dir="${APP_DIRECTORY}${dir}"
            break
        fi
    done

    # 编译合并后的模式
    if ls "${schema_dir}"/*.gschema.xml >/dev/null 2>&1; then
        # 将系统模式文件拷贝到临时目录
        cp -n "/opt/system/resource/glib-2.0/schemas"/* ${schema_dir}/ 2>/dev/null || true
        cp -n "/usr/share/glib-2.0/schemas"/* ${schema_dir}/ 2>/dev/null || true
        glib-compile-schemas --targetdir=${schema_dir} ${schema_dir} 2>/dev/null
        
        if [ -d "${app_schema_dir}" ]; then
            rm ${app_schema_dir}/gschemas.compiled 2>/dev/null
            cp ${schema_dir}/gschemas.compiled ${app_schema_dir}/ 2>/dev/null
        fi
    fi
}

compile_gschema

