. ./enums.sh
. ./libtest-core.sh

TRUE=true
FALSE=false

DEBUG="$FALSE"

for arg in "$@"; do
    if [ "$arg" == "-d" ]; then
        DEBUG="$TRUE"
        # 如果需要执行某些操作，可以放在这里
    fi
done

BOLD="$(tput bold 2>/dev/null || printf '')"
GREY="$(tput setaf 0 2>/dev/null || printf '')"
UNDERLINE="$(tput smul 2>/dev/null || printf '')"
RED="$(tput setaf 1 2>/dev/null || printf '')"
GREEN="$(tput setaf 2 2>/dev/null || printf '')"
YELLOW="$(tput setaf 3 2>/dev/null || printf '')"
BLUE="$(tput setaf 4 2>/dev/null || printf '')"
MAGENTA="$(tput setaf 5 2>/dev/null || printf '')"
NO_COLOR="$(tput sgr0 2>/dev/null || printf '')"

info() {
    printf '\n%s\n\n' "${BOLD}${YELLOW}! $*${NO_COLOR}"
}

warn() {
    printf '%s\n' "${YELLOW}! $*${NO_COLOR}"
}

error() {
    printf '%s\n\n' "${BOLD}${RED}>${NO_COLOR} ${BOLD}${RED}$*${NO_COLOR}" 
}

fn_simulate_ostree_system() {
    # 模拟ostree系统的环境 进行测试
    # 主要做的:
    # - 初始化一个ostree仓库:testos-repo
    # - 在osdata目录创建模拟一些系统文件 and 提交两次到testos-repo 仓库
    # - 复制osdata数据到osdata-devel,做出修改然后再次提交
    # - 用init-fs创建sysroot 和 os-init 和 syslinux
    # - 提交一个新的commit用空目录，标记初始分支（$BUILD_MASTER_BRACH）为EOL 重新定向到新分支（testos/buildmaster/newbranch）

    # 删除所有历史测试数据 重新创建测试
    if [ ! -d "$test_tmpdir" ]; then
        mkdir -p "$test_tmpdir"
        echo "创建测试目录: $test_tmpdir"
    else
        echo "创建测试目录: $test_tmpdir"
        sudo chattr -R -i $test_tmpdir/* >/dev/null 2>&1 || true
        chmod -R 777 $test_tmpdir/* >/dev/null 2>&1 || true
        rm -rf $test_tmpdir
        mkdir -p "$test_tmpdir"
    fi

    if [ ! -d "$PUSH_DIR" ]; then
        mkdir -p "$PUSH_DIR"
    fi

    # 创建ostree仓库 仓库类型 and boot类型 包括默认开启一个远程
    setup_os_repository "archive" "syslinux"

    cd ${test_tmpdir}
    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo remote add --set=gpg-verify=false testos $(cat httpd-address)/ostree/testos-repo

    ${CMD_PREFIX} ostree --repo=sysroot/ostree/repo pull testos $BUILD_MASTER_BRACH > /dev/null
    rev=$(${CMD_PREFIX} ostree --repo=sysroot/ostree/repo rev-parse $BUILD_MASTER_BRACH)
    echo "拉取分支:$BUILD_MASTER_BRACH 从远程仓库testos-repo and revision=${rev}"

    echo "部署分支:$BUILD_MASTER_BRACH"
    ${CMD_PREFIX} ostree admin deploy --karg=root=LABEL=MOO --karg=quiet --os=testos testos:$BUILD_MASTER_BRACH > /dev/null
    assert_has_dir sysroot/boot/ostree/testos-${bootcsum}
}

moniter_signal_success () {
    signal=$1
    
    tail -F -n $TAIL_LINE_NUM "$file_to_monitor" | while read -r line; do
        if echo -e "$line" | grep -q $signal" success = True"; then
            echo -e "$line \n"
            break
        fi

        if echo "$line" | grep -q 'ERROR'; then
            echo -e "\033[91m$line\033[0m"
        else
            echo "$line"
        fi
    done
}

moniter_signal_failed () {
    signal=$1
    
    tail -F -n $TAIL_LINE_NUM "$file_to_monitor" | while read -r line; do
        if echo -e "$line" | grep -q $signal" success = False"; then
            echo -e "$line \n"
            break
        fi

        if echo "$line" | grep -q 'ERROR'; then
            echo -e "\033[91m$line\033[0m"
        else
            echo "$line"
        fi
    done
}

moniter_string () {
    tail -F -n $TAIL_LINE_NUM "$file_to_monitor" | while read -r line; do
        if echo -e "$line" | grep "$1"; then
            echo -e "$line \n"
            break
        fi

        if echo "$line" | grep -q 'ERROR'; then
            echo -e "\033[91m$line\033[0m"
        else
            echo "$line"
        fi
    done
}

os_repository_new_commit ()
{
    boot_checksum_iteration=${1:-0}
    content_iteration=${2:-0}
    branch=${3:-$BUILD_MASTER_BRACH}
    export version=${4:-$(date "+%Y%m%d.${content_iteration}")}
    # echo "BOOT ITERATION: $boot_checksum_iteration"
    cd ${test_tmpdir}/osdata
    kver=3.6.0
    if test -f usr/lib/modules/${kver}/vmlinuz; then
        bootdir=usr/lib/modules/${kver}
    else
        if test -d usr/lib/ostree-boot; then
            bootdir=usr/lib/ostree-boot
        else
            bootdir=boot
        fi
    fi
    rm ${bootdir}/*
    kernel_path=${bootdir}/vmlinuz
    initramfs_path=${bootdir}/initramfs.img
    if [[ $bootdir != usr/lib/modules/* ]]; then
        kernel_path=${kernel_path}-${kver}
        initramfs_path=${bootdir}/initramfs-${kver}.img
    fi
    echo "new: a kernel ${boot_checksum_iteration}" > ${kernel_path}
    echo "new: an initramfs ${boot_checksum_iteration}" > ${initramfs_path}
    bootcsum=$(cat ${kernel_path} ${initramfs_path} | sha256sum | cut -f 1 -d ' ')
    export bootcsum
    if [[ $bootdir != usr/lib/modules/* ]]; then
        mv ${kernel_path}{,-${bootcsum}}
        mv ${initramfs_path}{,-${bootcsum}}
    fi

    echo "a new default config file" > usr/etc/a-new-default-config-file
    mkdir -p usr/etc/new-default-dir
    echo "a new default dir and file" > usr/etc/new-default-dir/moo
    
    echo "content iteration ${content_iteration}" > usr/bin/content-iteration

    echo -n "创建新分支:$branch and commit revision:"

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit  $metadata_string  --add-metadata-string "version=${version}" -b $branch -s "Build"
    cd ${test_tmpdir}
}

setup_os_boot_syslinux() {
    # Stub syslinux configuration
    mkdir -p sysroot/boot/loader.0
    ln -s loader.0 sysroot/boot/loader
    touch sysroot/boot/loader/syslinux.cfg
    # And a compatibility symlink
    mkdir -p sysroot/boot/syslinux
    ln -s ../loader/syslinux.cfg sysroot/boot/syslinux/syslinux.cfg
}

# A wrapper which also possibly disables xattrs for CI testing
ostree_repo_init() {
    repo=$1
    shift
    ${CMD_PREFIX} ostree --repo=${repo} init "$@"
    if test -n "${OSTREE_NO_XATTRS:-}"; then
        echo -e 'disable-xattrs=true\n' >> ${repo}/config
    fi
}

setup_os_repository () {
    mode=$1
    shift
    bootmode=$1
    shift
    bootdir=${1:-usr/lib/modules/3.6.0}

    oldpwd=`pwd`

    cd ${test_tmpdir}
    echo "创建远程仓库 $test_tmpdir/test-repo"
    mkdir testos-repo
    if test -n "$mode"; then
	      ostree_repo_init testos-repo --mode=${mode}
    else
	      ostree_repo_init testos-repo
    fi

    cd ${test_tmpdir}
    echo "模拟创建最小系统 $test_tmpdir/osdata"
    mkdir osdata
    cd osdata
    kver=3.6.0
    mkdir -p usr/bin ${bootdir} usr/lib/modules/${kver} usr/share usr/etc
    kernel_path=${bootdir}/vmlinuz
    initramfs_path=${bootdir}/initramfs.img
    # the HMAC file is only in /usr/lib/modules
    hmac_path=usr/lib/modules/${kver}/.vmlinuz.hmac
    # /usr/lib/modules just uses "vmlinuz", since the version is in the module
    # directory name.
    if [[ $bootdir != usr/lib/modules/* ]]; then
        kernel_path=${kernel_path}-${kver}
        initramfs_path=${bootdir}/initramfs-${kver}.img
    fi
    echo "a kernel" > ${kernel_path}
    echo "an initramfs" > ${initramfs_path}
    echo "an hmac file" > ${hmac_path}
    bootcsum=$(cat ${kernel_path} ${initramfs_path} | sha256sum | cut -f 1 -d ' ')
    export bootcsum
    # Add the checksum for legacy dirs (/boot, /usr/lib/ostree-boot), but not
    # /usr/lib/modules.
    if [[ $bootdir != usr/lib/modules/* ]]; then
        mv ${kernel_path}{,-${bootcsum}}
        mv ${initramfs_path}{,-${bootcsum}}
    fi

    echo "an executable" > usr/bin/sh
    echo "some shared data" > usr/share/langs.txt
    echo "a library" > usr/lib/libfoo.so.0
    ln -s usr/bin bin
cat > usr/etc/os-release <<EOF
NAME=TestOS
VERSION=42
ID=testos
VERSION_ID=42
PRETTY_NAME="TestOS 42"
EOF
    echo "a config file" > usr/etc/aconfigfile
    mkdir -p usr/etc/NetworkManager
    echo "a default daemon file" > usr/etc/NetworkManager/nm.conf
    mkdir -p usr/etc/testdirectory
    echo "a default daemon file" > usr/etc/testdirectory/test

    echo -n "第一次提交，分支:$BUILD_MASTER_BRACH and revision:"
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --add-metadata-string new_version=1.0.9 -b $BUILD_MASTER_BRACH -s "Build"

    # Ensure these commits have distinct second timestamps
    sleep 2
    echo "a new executable" > usr/bin/sh
    echo -n "第二次提交，分支:$BUILD_MASTER_BRACH and revision:"
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --add-metadata-string new_version=1.0.10 -b $BUILD_MASTER_BRACH -s "Build"

    cd ${test_tmpdir}
    rm -rf osdata-devel
    mkdir osdata-devel
    tar -C osdata -cf - . | tar -C osdata-devel -xf -
    cd osdata-devel
    mkdir -p usr/include
    echo "a development header" > usr/include/foo.h
    echo -n "第三次提交，分支:testos/buildmaster/x86_64-devel and revision:"
    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo commit --add-metadata-string new_version=1.0.9 -b testos/buildmaster/x86_64-devel -s "Build"
    
    # 更新summary文件
    # ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo summary -u

    ${CMD_PREFIX} ostree --repo=${test_tmpdir}/testos-repo fsck -q > /dev/null

    cd ${test_tmpdir}
    mkdir sysroot
    echo -n "初始化部署系统，位置$test_tmpdir/sysroot "
    export OSTREE_SYSROOT=sysroot
    ${CMD_PREFIX} ostree admin init-fs sysroot

    ${CMD_PREFIX} ostree admin os-init testos

    case $bootmode in
        "syslinux")
	    setup_os_boot_syslinux
            ;;
        "uboot")
	    setup_os_boot_uboot
            ;;
        *grub2*)
        setup_os_boot_grub2 "${bootmode}"
            ;;
        sysroot\.bootloader*)
        setup_os_boot_configured_bootloader "${bootmode}"
            ;;
    esac

    echo "创建http服务链接到当前仓库 $test_tmpdir/test-repo"
    cd ${test_tmpdir}
    mkdir ${test_tmpdir}/httpd
    cd httpd
    ln -s ${test_tmpdir} ostree

    $process_name >/dev/null 2>&1 &
    child_pid=$!
    trap "kill -9 $child_pid >/dev/null 2>&1 | true" EXIT
    # 延迟一段时间 等待服务开启 
    sleep 2

    echo $http_port > ${test_tmpdir}/httpd-port
    port=$(cat ${test_tmpdir}/httpd-port)
    echo "http://localhost:${port}" > ${test_tmpdir}/httpd-address
    cd ${oldpwd} 
}

if ! [ "$DEBUG" = "$TRUE" ]; then
    info "正在启动系统更新的后台服务...  参数：$@"
    $updater_program -r --sysroot=$test_tmpdir/sysroot -p --os=testos "$@" &
    sleep 3
fi