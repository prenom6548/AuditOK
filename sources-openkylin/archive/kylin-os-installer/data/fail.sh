#!/bin/bash

set -x

CONFIGFILE=/tmp/ky-installer.cfg
get_value() {
    local key=$1
    local value
    value=$(grep "^$key=" "$CONFIGFILE")
    echo "${value#*=}"
}

touch /var/log/installer/INSTALL_FAILED

new_sysroot_dir=$(get_value new-root)
if [[ -d $new_sysroot_dir/var/log/installer ]]; then
	cp -a /var/log/installer/*  $new_sysroot_dir/var/log/installer/
fi

sync
