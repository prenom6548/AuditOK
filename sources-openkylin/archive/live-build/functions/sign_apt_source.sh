#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2006-2012 Daniel Baumann <daniel@debian.org>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.
# 定义密钥参数

KEY_TYPE="RSA"
KEY_LENGTH="4096"
KEY_EXPIRE="0"
REAL_NAME="openkylin"
EMAIL_ADDRESS="live-build@openkylin.top"
COMMENT="Automatically generated GPG key"

# 生成GPG密钥函数
generate_gpg_key() {
    for fingerprint_key in $(get_fingerprint);do
        delete_gpg_key ${fingerprint_key}
    done
    local temp_config=$(mktemp -p .)
    cat << EOF > "$temp_config"
%echo Generating a basic OpenPGP key
Key-Type: $KEY_TYPE
Key-Length: $KEY_LENGTH
Subkey-Type: $KEY_TYPE
Subkey-Length: $KEY_LENGTH
Name-Real: $REAL_NAME
Name-Comment: $COMMENT
Name-Email: $EMAIL_ADDRESS
Expire-Date: $KEY_EXPIRE
%no-protection
%commit
%echo done
EOF
    gpg --batch --generate-key "$temp_config"
    rm "$temp_config"
}

# 获取密钥ID函数
get_key_id() {
    local key_id=$(gpg --list-secret-keys --keyid-format LONG "$EMAIL_ADDRESS" | awk '/sec/{print $2}' | cut -d '/' -f 2)
    echo "$key_id"
}

# 获取密钥指纹函数
get_fingerprint() {
    local fingerprint=$(gpg --list-secret-keys --with-colons "$EMAIL_ADDRESS" | awk -F: '$1 == "fpr" && $12 == "" {print $10; exit}')
    echo "$fingerprint"
}

# 对APT源进行签名函数
sign_apt_source() {
    local key_id="$1"
    local apt_dir="$2"
    if [ -d "$apt_dir" ]; then
        current_dir=$(pwd)
        cd "$apt_dir"
        rm -f InRelease Release.gpg
        apt-ftparchive release . > Release
        gpg --default-key "$key_id" --clearsign -o InRelease Release
        gpg --default-key "$key_id" -ab -o Release.gpg Release
        cd "$current_dir"
    fi
}

# 导出公钥函数
export_public_key() {
    local key_id="$1"
    local public_key_file="$2"
    gpg --armor --export "$key_id" > "$public_key_file"
    echo "公钥已导出到 $public_key_file"
}

# 导出私钥函数
export_private_key() {
    local key_id="$1"
    local private_key_file="$2"
    gpg --armor --export-secret-keys "$key_id" > "$private_key_file"
    echo "私钥已导出到 $private_key_file"
}

# 删除GPG密钥函数
delete_gpg_key() {
    local fingerprint="$1"
    gpg --batch --yes --delete-secret-keys "$fingerprint"
    gpg --batch --yes --delete-keys "$fingerprint"
    echo "GPG密钥已成功删除"
}

# 导入私钥函数
import_private_key() {
    local private_key_file="$1"
    if [ -f "$private_key_file" ]; then
        apt-key add "$private_key_file"
        echo "私钥已成功导入"
    else
        echo "私钥文件不存在"
    fi
}

# 生成GPG密钥
#generate_gpg_key
#KEY_ID=$(get_key_id)
#FINGERPRINT=$(get_fingerprint)

# 对APT源进行签名
#sign_apt_source "$KEY_ID"

# 导出私钥
#export_private_key "$KEY_ID"

# 删除GPG密钥
#delete_gpg_key "$FINGERPRINT"

# 导入私钥
#import_private_key
