#!/bin/bash

. ./libtest.sh

test_large_os_download() {
    fn_simulate_ostree_system

    info "测试下载超大文件 3.0G以上，花费时间较长 主要花费时间在commit内容上"

    dd if=/dev/urandom of=${test_tmpdir}/osdata/random_data.bin bs=1G count=100
    new_brach="testos/buildmaster/newbranch1"
    os_repository_new_commit 1 1 $new_brach

    mkdir empty
    echo -n "当前系统分支:$BUILD_MASTER_BRACH，配置切换到新分支$new_brach revision:"
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --tree=dir=$(pwd)/empty --add-metadata-string "ostree.endoflife-rebase=testos/buildmaster/newbranch1" -b testos/buildmaster/newbranch -s "EOL redirect to new branch"

    echo ""
    echo "检查更新"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null
    moniter_signal_success $update_finished_signal

    echo "部署下载"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$download_method > /dev/null
    moniter_signal_success $download_finished_signal

    echo "部署内容"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$deploy_method > /dev/null
    moniter_signal_success $deploy_finished_signal

    # 部署完成后的检查
    assert_file_has_content sysroot/boot/loader/entries/ostree-2-testos.conf "${bootcsum}"
    rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0/usr/bin/content-iteration "1"

    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0.origin "newbranch"
}

test_large_os_download