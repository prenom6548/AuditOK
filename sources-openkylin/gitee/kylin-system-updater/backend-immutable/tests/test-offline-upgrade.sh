#!/bin/bash

. ./libtest.sh

test_delta_upgrade_progress() {
    rm -rf  files repo temp-repo out.txt tmpsubdir repo2 deltadir repo3 deltadir | true
    bindatafiles="bash true ostree"
    morebindatafiles="false ls"

    # 创建开始的仓库
    mkdir repo
    ostree_repo_init repo --mode=archive

    # 创建一些测试文件
    mkdir files
    for bin in ${bindatafiles}; do
        cp $(which ${bin}) files
    done

    # ----------------------------------------------服务端仓库 远程仓库-------------------------------------------
    # 进行第一次提交 commit 到仓库
    ${CMD_PREFIX} ostree --repo=repo commit -b test -s test --tree=dir=files
    origrev=$(${CMD_PREFIX} ostree --repo=repo rev-parse test)

    # 进行第二次提交 到仓库
    cp /usr/bin/apt-get files
    ${CMD_PREFIX} ostree --repo=repo commit -b test -s "原始版本" --tree=dir=files
    newrev=$(${CMD_PREFIX} ostree --repo=repo rev-parse test)

    # 第三次提交 到仓库    
    cp /usr/bin/bash files
    ${CMD_PREFIX} ostree --repo=repo commit --add-metadata-string=update-name="dsdsdd" -b test1 -s "最新更新"  --tree=dir=files
    sedrev=$(${CMD_PREFIX} ostree --repo=repo rev-parse test1)

    ${CMD_PREFIX} ostree --repo=repo static-delta generate --from=${origrev}  --to=${newrev} 
    # 产生delta文件
    ${CMD_PREFIX} ostree --repo=repo static-delta generate --from=${newrev}  --to=${sedrev} 

    ${CMD_PREFIX} ostree --repo=repo summary -u

    # ---------------------------------------------创建一个空仓库 + 离线数据-----------------------------------------
    mkdir repo2 && ostree_repo_init repo2 --mode=archive

    ${CMD_PREFIX} ostree --repo=repo2 remote add --set=gpg-verify=false repo file://$(pwd)/repo

    # 拉取将要升级的源数据 
    ${CMD_PREFIX} ostree --repo=repo2 pull --depth=-1 --commit-metadata-only repo test
    ${CMD_PREFIX} ostree --repo=repo2 pull --depth=-1 --commit-metadata-only repo test1

    # 放置delta差分数据
    cp -r repo/deltas repo2/

    # 复制服务都的summary文件 因为服务端包含完整的commit和分支数据 
    cp -r repo/summary repo2/
    # ${CMD_PREFIX} ostree --repo=repo2 summary -u

    #-----------------------------------------------本地测试设备------------------------------------------------
    # 创建空仓库
    mkdir repo3 && ostree_repo_init repo3 --mode=bare

    # 模拟 从服务端仓库拉取最开始的版本
    ${CMD_PREFIX} ostree --repo=repo3 remote add --set=gpg-verify=false repo file://$(pwd)/repo
    ${CMD_PREFIX} ostree --repo=repo3 pull repo test@${origrev}
    ${CMD_PREFIX} ostree --repo=repo3 remote delete repo
    rm -rf repo

    # 配置本地仓库连接到 u盘源仓库
    ${CMD_PREFIX} ostree --repo=repo3 remote add --set=gpg-verify=false repo file://$(pwd)/repo2

    # 检查更新阶段
    # ${CMD_PREFIX} ostree --repo=repo3 pull --depth=-1 --commit-metadata-only repo test
    # ${CMD_PREFIX} ostree --repo=repo3 pull --depth=-1 --commit-metadata-only repo test1

    # 下载阶段
    ${CMD_PREFIX} ostree --repo=repo3 pull --require-static-deltas --depth=-1 repo test@${newrev}
    ${CMD_PREFIX} ostree --repo=repo3 pull --require-static-deltas --depth=-1 repo test1@${sedrev}

    echo "-------------"

    ${CMD_PREFIX} ostree --repo=repo3 log repo:test
    echo "-------------"
}


test_delta_upgrade() {
    fn_simulate_ostree_system

    OS_DATA="/tmp/ostree-auto-test/osdata"
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo static-delta generate --to=$BUILD_MASTER_BRACH
    # 服务器 提交新的可升级补丁
    cp /usr/bin/bash $OS_DATA
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo static-delta generate --to=$BUILD_MASTER_BRACH

    cp /usr/bin/apt-get $OS_DATA
    echo "1" > $OS_DATA/usr/bin/content-iteration
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    sedrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    # 产生delta文件
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo static-delta generate --to=$BUILD_MASTER_BRACH
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo summary -u

    # ---------------------------------------------构建离线更新源-----------------------------------------
    mkdir ${test_tmpdir}/offline-repo && ostree_repo_init ${test_tmpdir}/offline-repo --mode=archive

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/offline-repo remote add --set=gpg-verify=false testos file://${test_tmpdir}/testos-repo

    # 拉取将要升级的源数据  需要拉取所有的元数据
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/offline-repo pull --depth=-1 --commit-metadata-only testos $BUILD_MASTER_BRACH

    # 防止源数据
    cp -r ${test_tmpdir}/testos-repo/deltas ${test_tmpdir}/offline-repo/

    cp ${test_tmpdir}/testos-repo/summary ${test_tmpdir}/offline-repo/
    # ----------------------------------------------------------------------------------------------------

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote delete testos

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote add --set=gpg-verify=false testos file://${test_tmpdir}/offline-repo

    echo "testos:${BUILD_MASTER_BRACH}@${sedrev}" > $PUSH_FILE

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
    rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0/usr/bin/content-iteration "1"
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0.origin "newbranch"
}


test_all_upgrade() {
    fn_simulate_ostree_system

    OS_DATA="/tmp/ostree-auto-test/osdata"

    cp /usr/bin/bash $OS_DATA
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    newrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    cp /usr/bin/apt-get $OS_DATA
    echo "1" > $OS_DATA/usr/bin/content-iteration
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $BUILD_MASTER_BRACH -s "Build" --tree=dir=$OS_DATA
    sedrev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo summary -u

    # ---------------------------------------------构建离线更新源-----------------------------------------
    mkdir ${test_tmpdir}/offline-repo && ostree_repo_init ${test_tmpdir}/offline-repo --mode=archive

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/offline-repo remote add --set=gpg-verify=false testos file://${test_tmpdir}/testos-repo

    # 拉取将要升级的源数据  需要拉取所有的元数据
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/offline-repo pull --depth=1 testos $BUILD_MASTER_BRACH@${sedrev}

    # cp ${test_tmpdir}/testos-repo/summary ${test_tmpdir}/offline-repo/
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/offline-repo summary -u
    # ----------------------------------------------------------------------------------------------------

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote delete testos

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote add --set=gpg-verify=false testos file://${test_tmpdir}/offline-repo

    echo "testos:${BUILD_MASTER_BRACH}@${sedrev}" > $PUSH_FILE

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
    rev=$(${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo rev-parse $BUILD_MASTER_BRACH)
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0/usr/bin/content-iteration "1"
    assert_file_has_content sysroot/ostree/deploy/testos/deploy/${rev}.0.origin "newbranch"
}

test_delta_upgrade
test_all_upgrade