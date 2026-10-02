#!/bin/bash

function is_root() {
    if [ "$(id -u)" -eq 0 ]; then
        return 0
    else
        return 1
    fi
}

# 提醒：大部分不涉及：如果你的代码会编出动态库，并且要对动态库中的代码进行测试，要先构建带覆盖率参数的动态库，才能统计到动态库代码的覆盖率，请注意
function replace_apt_source(){
    local apt_source="deb http://ppa.launchpad.dev/zhaominyong/v11-2603-preview/kylin-desktop-v11 v11-2603-preview main"
    if ! grep -qF "$apt_source" /etc/apt/sources.list; then
        chattr -i /etc/apt/sources.list
        sed -i "1i\\$apt_source" /etc/apt/sources.list
        chattr +i /etc/apt/sources.list
    fi
    apt update
}

if is_root; then
    echo "当前用户是 root，执行需要超级用户权限的操作..."
    replace_apt_source

    apt install -y build-essential libglib2.0-dev libostree-dev libxml2-dev pkg-config cmake ninja-build libdbus-glib-1-dev nlohmann-json3-dev libdbus-1-dev libcurl4-gnutls-dev zlib1g-dev libelf-dev liblz4-dev liblzma-dev libdeflate-dev libzstd-dev uuid-dev libgtest-dev crun libyajl-dev patchelf xdelta3 libfuse-dev liblzo2-dev squashfs-tools lcov kaiming squashfuse python3-pip

    # 此处是从指定内网下载gcovr
    pip install gcovr==7.1 -i https://maven.kylinos.cn/repository/pypi-group/simple/ --break-system-packages

    systemctl start polkit

    systemctl stop org.kylin.kaiming
    systemctl start org.kylin.kaiming

    # 配置kaiming仓库, 指向指定内网的kaiming仓库
    kaiming remote modify kaiming-repo --repo=http://10.42.181.67:8002/KM/repo-v11

    kaiming update
else
    echo "当前用户不是 root，跳过需要超级用户权限的操作..."
fi

cd "$(dirname "$0")" || exit 1 

if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build  # 强制删除build目录及所有内容
fi

echo "running cmake"
cmake -Bbuild -DENABLE_COVERAGE=ON
if [ $? -ne 0 ]; then
    echo "Error: cmake failed."
    exit -1
fi

echo "Running make..."
cd build
make -j4
if [ $? -ne 0 ]; then
    echo "Error: make failed."
    exit -1
fi
echo "Build successful!"
exit 0

