#!/bin/bash
updater_program=$(pwd)/../kylin-system-updater

. ./libtest.sh

fn_simulate_ostree_system() {
    # 模拟ostree系统的环境 进行测试
    # 主要做的:
    # - 初始化一个ostree仓库:testos-repo
    # - 在osdata目录创建模拟一些系统文件 and 提交两次到testos-repo 仓库
    # - 复制osdata数据到osdata-devel,做出修改然后再次提交
    # - 用init-fs创建sysroot 和 os-init 和 syslinux
    # - 提交一个新的commit用空目录，标记初始分支（$BUILD_MASTER_BRACH）为EOL 重新定向到新分支（testos/buildmaster/newbranch）

    # 创建ostree仓库 仓库类型 and boot类型 包括默认开启一个远程
    setup_os_repository "archive" "syslinux"

    rm $PUSH_FILE | true

    cd ${test_tmpdir}
    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote add --set=gpg-verify=false testos $(cat httpd-address)/ostree/testos-repo

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo pull testos $BUILD_MASTER_BRACH > /dev/null
    rev=$(${CMD_PREFIX} ostree --repo=sysroot/ostree/repo rev-parse $BUILD_MASTER_BRACH)
    echo "拉取分支:$BUILD_MASTER_BRACH 从远程仓库testos-repo and revision=${rev}"

    echo "部署分支:$BUILD_MASTER_BRACH"
    ${CMD_PREFIX} ostree admin deploy --karg=root=LABEL=MOO --karg=quiet --os=testos testos:$BUILD_MASTER_BRACH > /dev/null
    assert_has_dir sysroot/boot/ostree/testos-${bootcsum}
}

fn_simulate_ostree_system

# 启动系统更新服务
$updater_program -r -d --sysroot=$test_tmpdir/sysroot --os=testos &

new_brach="testos/buildmaster/newbranch1"
os_repository_new_commit 1 1 $new_brach

echo -n "当前系统分支:$BUILD_MASTER_BRACH，配置切换到新分支$new_brach revision:"

mkdir empty
${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --tree=dir=$(pwd)/empty --add-metadata-string "ostree.endoflife-rebase=testos/buildmaster/newbranch1" -b testos/buildmaster/newbranch -s "EOL redirect to new branch"

sleep 10000000