#!/bin/bash
export  GI_TYPELIB_PATH=/opt/update-module/current/usr/lib/aarch64-linux-gnu/girepository-1.0
export  LD_LIBRARY_PATH=/opt/update-module/current/usr/lib/aarch64-linux-gnu

. ./libtest.sh

download_test_data() {
    SOURCE_DIR="/home/x/test-data"

    # 遍历所有 .deb 文件并解压
    for deb in "$SOURCE_DIR"/*.deb; do
        echo "Extracting $deb..."
        dpkg -x "$deb" "$OS_DATA"
    done

    mkdir -p $OS_DATA/var/lib/dpkg/info/ $OS_DATA/usr/share/update-manifests/hooks/
    cp $SOURCE_DIR/*.list $OS_DATA/var/lib/dpkg/info/
    cp $SOURCE_DIR/*.sh $OS_DATA/usr/share/update-manifests/hooks/
    cp /var/lib/dpkg/status $OS_DATA/var/lib/dpkg/

    rm -rf $OS_DATA/usr/etc
    mv $OS_DATA/etc/ $OS_DATA/usr/
}

pust_manifests() {
    mkdir -p $OS_DATA/usr/share/update-manifests/
cat > $OS_DATA/usr/share/update-manifests/kylin-system-updater.manifest.json <<EOF
{
    "package": "kylin-system-updater",
    "exclude_patterns": [
        "/usr/share/doc/**",
        "/usr/share/update-manifests/**"
    ],
    "extra_files": {
            "move": {
                "/usr/lib/systemd/system/kylin-system-updater.service": "/opt/system/lib/systemd/"
            }
    },
    "failure_rate": 1,
    "post_hooks": [
        {
            "command": "/usr/sbin/kyexectl -s -r",
            "value": "/opt/update-module/versions",
            "priority": 0
        },
        {
            "command": "python3",
            "value": " /usr/share/update-manifests/hooks/set-netctl.py",
            "priority": 1
        },
        {
            "command": "pkill -f",
            "value": "/opt/update-module/current/usr/share/kylin-system-updater/kylin-upgrade-strategies"
        },
        {
            "command": "pkill -f",
            "value": "/usr/share/kylin-system-updater/kylin-upgrade-strategies"
        }
    ]
}
EOF

cat > $OS_DATA/usr/share/update-manifests/kylin-update-frontend.json <<EOF
{
    "package": "kylin-update-frontend",
    "version": "3.1.2-1",
    "hooks": {
        "pre_checkout": ["/usr/share/update-manifests/hooks/kylin-system-updater/pre-checkout.sh"],
        "post_checkout": ["/usr/share/update-manifests/hooks/kylin-system-updater/post-checkout.sh"] 
    },
    "post_actions": [
        {
            "command": "/usr/bin/touch",
            "value": "/tmp/aptpppdpdp",
            "priority": 999
        },
        {
            "command": "/usr/share/update-manifests/hooks/test.sh",
            "priority": 999
        }
    ]
}
EOF
cat > $OS_DATA/usr/share/update-manifests/gir1.2-ostree-1.0.json <<EOF
{
    "package": "gir1.2-ostree-1.0",
    "dpkg_arch_suffix": true
}
EOF
cat > $OS_DATA/usr/share/update-manifests/libostree-1-1.json <<EOF
{
    "package": "libostree-1-1",
    "dpkg_arch_suffix": true
}
EOF
}


test_new_branch_and_commit() {
    fn_simulate_ostree_system
    new_brach="testos/buildmaster/newbranch1"
    os_repository_new_commit 1 1 $new_brach

    download_test_data
    pust_manifests

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit $metadata_string --add-metadata-string new_version=1.0.11 -b $new_brach -s "Build" --tree=dir=$OS_DATA

    push_content="testos:${new_brach}"

    info "测试：推送更新内容:$push_content"

    echo "$push_content" > $PUSH_FILE

    # kill -9 $(lsof -ti :8889)
    # sleep 100000
    echo "检查更新"
    gdbus call --system --dest $bus_name --object-path $object_path --method $interface_name.$update_method > /dev/null
    moniter_string "$update_finished_signal success = False , upgrade_group = \['kylin-system-updater'\]"

    exit 0
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

test_new_branch_and_commit