#!/bin/bash

. ./libtest.sh

test_only_branch() {
    fn_simulate_ostree_system

    new_brach="testos/buildmaster/newbranch1"
    os_repository_new_commit 1 1 $new_brach
    push_content="testos:${new_brach}"

    info "测试：推送更新内容,仅包含分支信息，推送内容：$push_content"

    echo "${push_content}" > $PUSH_FILE

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

test_new_branch_and_commit() {
    fn_simulate_ostree_system
    new_brach="testos/buildmaster/newbranch1"
    os_repository_new_commit 1 1 $new_brach
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA

    push_content="testos:${new_brach}@$newrev"

    info "测试：推送更新内容:$push_content"

    echo "$push_content" > $PUSH_FILE

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
    # rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0/usr/bin/content-iteration "1"

    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0.origin "newbranch"
}

test_new_branch_and_old_commit() {
    echo "测试部署新分支 并且提交的commit在"
    fn_simulate_ostree_system
    new_brach="testos/buildmaster/newbranch1"

    # 再次提交保持不是最新的
    os_repository_new_commit 1 1 $new_brach
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)

    os_repository_new_commit 1 1 $new_brach
    push_content="testos:${new_brach}@$newrev"

    info "测试：推送更新内容:$push_content"

    echo "$push_content" > $PUSH_FILE


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
    # rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0/usr/bin/content-iteration "1"

    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0.origin "newbranch"
}



test_old_branch_and_commit() {
    fn_simulate_ostree_system

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.12 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA

    push_content="testos:${BUILD_MASTER_BRACH}@$newrev"
    info "测试分支@commit号，例如推送更新内容:$push_content"

    echo $push_content > $PUSH_FILE

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
    # rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $new_brach)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0/usr/bin/sh "executable"
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${newrev}.0.origin "newbranch"
}
test_on_push_update() {
    info "测试 没有推送更新文件的情况并且不切换分支，直接提示最新的。"
    if [ -f "$PUSH_FILE" ]; then
        rm $PUSH_FILE | true
    fi

    fn_simulate_ostree_system

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    echo ""
    echo "测试：推送更新内容，升级到当前分支 $BUILD_MASTER_BRACH @$newrev"

    echo ""
    echo "检查更新"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null

    moniter_string "$update_finished_signal success = True , upgrade_group = \[\]"
}
test_exeption_push() {
    fn_simulate_ostree_system

    new_brach="testos/buildmaster/newbranch1"
    os_repository_new_commit 1 1 $new_brach
    push_content="${new_brach}"

    info "测试：推送更新内容异常情况，推送内容：$push_content"

    echo "${push_content}" > $PUSH_FILE

    echo "检查更新"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null

    # 提示为最新
    moniter_string "$update_finished_signal success = True , upgrade_group = \[\]"

    info "测试：推送错误的分支。"
    push_content="testos:dddd"
    echo "${push_content}" > $PUSH_FILE
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null
    moniter_string "$update_finished_signal success = False , upgrade_group = \[''\], error_code = #0003"

    # info "测试：推送错误的commit号。"
    # push_content="testos:${new_brach}@dsdsdsdsdsds"
    # echo "${push_content}" > $PUSH_FILE
    # gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null
    # moniter_string "$update_finished_signal success = False , upgrade_group = \[''\]"
}

test_on_push_update

test_only_branch

test_new_branch_and_commit

test_old_branch_and_commit

test_exeption_push

test_new_branch_and_old_commit