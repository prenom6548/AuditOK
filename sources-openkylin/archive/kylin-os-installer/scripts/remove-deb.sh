#!/bin/bash
set -e

. /tmp/utils

do_inchroot_remove_deb() {
  run_scripts_dir "${IN_TARGET}"/in_chroot_remove_deb
}

do_inchroot_remove_deb
msg "运行remove deb 脚本结束"
