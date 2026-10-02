# repo_checkout.py
# /*
#  * Copyright (C) 2020, KylinSoft Co., Ltd.
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  */
import os
import logging
from typing import Dict
from gi.repository import GLib, OSTree

def checkout_files_partly(repo_obj, trans, pull_dirs, pull_files, put_dir) -> Dict[str, str]:
    """
    部分检出指定目录和文件到目标目录，并返回失败文件的字典。
    对于 /etc、/lib、/bin、/sbin 开头的路径，从 /usr/... 路径下检出，
    但放置路径保持原始路径结构。
    """
    failed_files: Dict[str, str] = {}

    # 映射规则
    path_prefix_map = {
        "/etc": "/usr/etc",
        "/lib": "/usr/lib",
        "/bin": "/usr/bin",
        "/sbin": "/usr/sbin",
        "/opt": "/var/opt"
    }

    def _map_path(path: str) -> str:
        for prefix, mapped in path_prefix_map.items():
            if path == prefix or path.startswith(prefix + "/"):
                return path.replace(prefix, mapped, 1)
        return path

    # 检查是否有内容
    if not (pull_dirs or pull_files):
        logging.info("No files or directories to checkout.")
        return failed_files

    os.makedirs(put_dir, exist_ok=True)

    # 映射后的子路径用于 pull 操作
    all_origin_paths = pull_dirs + pull_files
    all_mapped_paths = [_map_path(p) for p in all_origin_paths]

    g_options = {
        "n-network-retries": GLib.Variant("u", 5),
        "flags": GLib.Variant("i", OSTree.RepoPullFlags.NONE),
        "depth": GLib.Variant("i", 1),
        "max-speed-limit-kilobyte": GLib.Variant("u",os.environ.get("DOWNLOAD_SPEED_LIMIT",0)),\
        "override-commit-ids": GLib.Variant("as",[trans.new_revision]),
        "subdirs": GLib.Variant("as", all_mapped_paths),
        "refs": GLib.Variant("as", [trans.available_refs])
    }

    repo_obj.pull_with_options(
        trans.available_repo,
        GLib.Variant("a{sv}", g_options),
        None,
        None
    )

    postinst_opts = OSTree.RepoCheckoutAtOptions()
    postinst_opts.overwrite_mode = int(OSTree.RepoCheckoutOverwriteMode.ADD_FILES)
    postinst_opts.force_copy = True

    def _checkout_subpath(origin_path: str, target_dir: str) -> bool:
        """根据映射路径尝试检出，失败则记录"""
        mapped_path = _map_path(origin_path)
        postinst_opts.subpath = mapped_path
        os.makedirs(target_dir, exist_ok=True)
        try:
            repo_obj.checkout_at(postinst_opts, 0, target_dir, trans.new_revision, None)
            return True
        except Exception as excep:
            failed_files[origin_path] = str(excep)
            logging.warning("Checkout failed for '%s' (mapped as '%s'): %s",
                            origin_path, mapped_path, excep)
            return False

    # Checkout 目录
    if pull_dirs:
        logging.info("Start checking out %d directories to %s", len(pull_dirs), put_dir)
        for dir_path in pull_dirs:
            target_dir = os.path.join(put_dir, dir_path.lstrip('/'))
            _checkout_subpath(dir_path, target_dir)

    # Checkout 文件
    if pull_files:
        logging.info("Start checking out %d files to %s", len(pull_files), put_dir)
        for file_path in pull_files:
            target_dir = os.path.join(put_dir, os.path.dirname(file_path).lstrip('/'))
            _checkout_subpath(file_path, target_dir)

    return failed_files
