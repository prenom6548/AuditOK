#!/bin/bash
uid=$(id -u)

if [ $# -ne 1 ]; then
    echo "请提供应用名称作为参数"
    exit 1
fi
target=$1

function is_app_running() {
    kaiming ps | tail -n +2 | awk '{print $1}' | grep -q "^$target$"
}

function run_test() {
    kaiming run "$target" > /dev/null 2>&1 &
    sleep 20

    if is_app_running "$target"; then
        echo "应用 '$target' 启动成功"
    else
        echo "应用 '$target' 启动失败"
        exit 1
    fi

    # echo "模拟挂载 U 盘..."
    sudo mkdir -p /media/usb-test
    sudo mount -t tmpfs -o size=10M tmpfs /media/usb-test
    sleep 10

    # echo "🔍 检查容器内是否看到 U 盘挂载..."
    kaiming enter "$target" -- ls /media | grep -q "usb-test"
    if [ $? -eq 0 ]; then
        echo "容器内已识别挂载，传播正常"
    else
        echo "容器内未识别挂载，传播失败"
        kaiming kill $target
        exit 1
    fi
}

run_test
kaiming kill $target
sudo umount /media/usb-test
sudo rmdir /media/usb-test

