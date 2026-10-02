#!/bin/bash

. ./libtest.sh

test_deploy_limit() {
    
    fn_simulate_ostree_system
    if [ -f "$PUSH_FILE" ]; then
        rm $PUSH_FILE | true
    fi

    for ((i=1; i<=$1; i++))
    do
        y=$((i - 1))

        if [ $y -eq 0 ]; then
            y=""
        fi
        echo ""
        echo "测试：从当前分支切换到新分支部署升级"

        new_brach="testos/buildmaster/newbranch$i"
        os_repository_new_commit 1 1 $new_brach

        mkdir empty$i
        echo -n "当前系统分支:$BUILD_MASTER_BRACH，配置切换到新分支$new_brach revision:"
        ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --tree=dir=$(pwd)/empty$i --add-metadata-string "ostree.endoflife-rebase=testos/buildmaster/newbranch$i" -b testos/buildmaster/newbranch$y -s "EOL redirect to new branch"

        echo ""
        echo "Ostree 测试环境创建成功, 开始系统更新功能测试"
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
    done
}

test_deploy_limit 1
