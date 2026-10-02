# update.py
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
import json
import shutil
import logging
from typing import Dict,List,Tuple
from pydantic import BaseModel

from .partial_checkout import checkout_files_partly
from .dpkg import get_versions_from_status
from .hooks import execute_hooks,execute_netctl
from .config import VERSIONS_DIR, MANIFESTS_DIR, CURRENT_PROGRESS,VERSION_MANIFEST_FILE
from .manifest import (
    PostAction,
    ComponentManifest,
    parse_manifests,
    collect_installed_files_from_manifests,
    get_all_dpkg_list_files,
    get_move_files_from_manifests,
    get_all_netctl_files,
    get_all_post_hooks,
    get_failure_rate,
    handle_move_operations,
    get_remapped_files
)

DPKG_STATUS_PATH = "/var/lib/dpkg/status"
VERSION_ID_FILE_NAME = "HEAD"

class VersionConfig(BaseModel):
    pull_ref: str = None               # 拉取来源分支
    commit_id: str = None              # 当前版本commit id
    packages: Dict[str, str] = {}
    moved_files: Dict[str, str] = {}
    executed_hooks: List[PostAction] = []     # 所有更新组件自升级完成后 需要做的处理，例如重启服务
    failed_files: Dict[str, str] = {}          # 记录那些文件都拉取失败了
    manifests: Dict[str, ComponentManifest] = {}  # 每个组件的manifest描述

def get_current_version() -> Tuple[str, VersionConfig]:
    version_config = VersionConfig()

    version_manifest_path = os.path.join(CURRENT_PROGRESS, VERSION_MANIFEST_FILE)
    if os.path.exists(version_manifest_path):
        try:
            with open(version_manifest_path, 'r') as f:
                data = json.load(f)
            version_config = VersionConfig(**data)
        except (OSError, json.JSONDecodeError) as e:
            logging.error(f"Failed to load version manifest: {e}")

    return version_config.commit_id, version_config

def _clean_old_versions(new_version_id: str, old_version_id: str):
    for version_name in os.listdir(VERSIONS_DIR):
        version_path = os.path.join(VERSIONS_DIR, version_name)
        if version_name not in {new_version_id, old_version_id} and os.path.isdir(version_path):
            logging.info("Deleting outdated version: %s", version_name)
            shutil.rmtree(version_path)

def _clean_old_config(new_moved_files: Dict[str, str], old_moved_files: Dict[str, str]):
    """
    清理旧版本遗留的配置文件。
    
    规则：
    - 如果旧版本迁移的源文件在新版本中已不存在，删除对应的目标文件。
    - 如果源文件存在，但目标路径变化，删除旧目标文件。
    - 如果源文件和目标路径均未变化，保留目标文件。
    """
    for src_file, old_dst_file in old_moved_files.items():
        new_dst_file = new_moved_files.get(src_file)

        if new_dst_file is None:
            # 新版本已无此迁移项，直接删除旧文件
            if os.path.exists(old_dst_file):
                logging.info("Deleting outdated config file: %s", old_dst_file)
                os.remove(old_dst_file)
        elif new_dst_file != old_dst_file:
            # 迁移路径发生变化，删除旧目标文件
            if os.path.exists(old_dst_file):
                logging.info("Deleting outdated config file due to path change: %s",old_dst_file)
                os.remove(old_dst_file)
        else:
            # 迁移路径未变化，保留文件
            logging.info("Keeping valid config file: %s", old_dst_file)

def _activate_new_version(version_path: str) -> None:
    """
    激活指定版本目录，通过替换 CURRENT_PROGRESS 链接指向该版本。

    参数:
        version_path (str): 新版本目录的路径，必须为有效目录。

    逻辑说明:
        - 校验版本路径有效性。
        - 清理当前 CURRENT_PROGRESS，支持符号链接、目录或普通文件情况。
        - 将当前版本目录重命名备份。
        - 将新版本目录重命名为 CURRENT_PROGRESS，实现版本切换。
    """
    if not os.path.isdir(version_path):
        logging.error("Invalid version path: %s", version_path)
        return

    # 清理当前链接（符号链接、目录或文件）
    if os.path.islink(CURRENT_PROGRESS):
        os.unlink(CURRENT_PROGRESS)  # 删除符号链接
    elif os.path.isdir(CURRENT_PROGRESS):
        version_file = os.path.join(CURRENT_PROGRESS, VERSION_ID_FILE_NAME)
        with open(version_file, 'r', encoding='utf-8') as f:
            current_commit_id = f.read().strip()
        old_move_post = os.path.join(VERSIONS_DIR, current_commit_id)
        os.rename(CURRENT_PROGRESS, old_move_post)

    try:
        # 重命名新版本目录为 CURRENT_PROGRESS，完成切换
        os.rename(version_path, CURRENT_PROGRESS)
        logging.info("Activated version: %s", CURRENT_PROGRESS)
    except OSError as e:
        logging.error("Failed to activate version '%s': %s", version_path, e)

def _should_upgrade(new_version_id: str) -> bool:
    """
    判断是否需要执行升级逻辑。

    :param new_version_id: 新版本的 commit ID
    :return: 如果需要升级，返回 True；否则返回 False
    """
    progress_path = CURRENT_PROGRESS
    version_file = os.path.join(progress_path, VERSION_ID_FILE_NAME)

    if not os.path.isdir(progress_path):
        logging.info("Upgrade needed: version path does not exist: %s", progress_path)
        return True

    if os.path.islink(progress_path):
        try:
            current_commit_id = os.path.basename(os.readlink(progress_path))
            if current_commit_id == new_version_id:
                logging.info("No upgrade needed: already running version %s", new_version_id)
                return False
        except OSError as e:
            logging.warning(f"Failed to read current symlink: {e}")
    else:
        try:
            with open(version_file, 'r', encoding='utf-8') as f:
                current_commit_id = f.read().strip()

            if current_commit_id == new_version_id:
                logging.info("No upgrade needed: already running version %s", new_version_id)
                return False

            logging.info("Upgrade needed: current version %s != new version %s", current_commit_id, new_version_id)
        except (OSError, FileNotFoundError) as e:
            logging.warning("Failed to read current version file '%s': %s", version_file, e)

    return True

def add_new_version(repo_obj, trans,version_root) -> VersionConfig:
    version_config = VersionConfig(
        pull_ref=trans.available_refs,
        commit_id=trans.new_revision,
    )

    logging.info("Checking out update manifests...")
    failed_file = checkout_files_partly(repo_obj, trans, [MANIFESTS_DIR], [], version_root)
    version_config.failed_files.update(failed_file)
    version_config.manifests = parse_manifests(version_root)
    if not version_config.manifests:
        logging.warning("No manifests found, aborting self upgrade add new version addition.")
        return None

    logging.info("Checking out dpkg list and status...")
    dpkg_files = get_all_dpkg_list_files(version_config.manifests)
    dpkg_files.append(DPKG_STATUS_PATH) # 追加拉取status文件
    failed_file = checkout_files_partly(repo_obj, trans, [], dpkg_files, version_root)
    version_config.failed_files.update(failed_file)

    # 解析status文件，记录每个软件包的版本信息
    packages_list = list(version_config.manifests.keys())
    status_file = os.path.join(version_root, DPKG_STATUS_PATH.lstrip('/'))
    version_config.packages = get_versions_from_status(packages_list,status_file)

    logging.info("Starting checkout package files:%s",version_config.packages)
    all_files = collect_installed_files_from_manifests(version_root,version_config.manifests)
    failed_file = checkout_files_partly(repo_obj, trans, [], all_files, version_root)
    version_config.failed_files.update(failed_file)

    # 检查是否超过一定比例的失败 超过时 认为自更新失败
    failed_limit = int(len(all_files) * get_failure_rate(version_config.manifests))
    if len(failed_file) > failed_limit:
        logging.warning(f"Failed files({len(failed_file)}) larger than {failed_limit} files checked out, aborting...",)
        return None

    return version_config

def _add_version_id_file(version_path: str) -> None:
    """
    在指定版本目录下创建一个包含 commit ID 的标识文件。
    
    :param version_path: 版本目录的路径，该目录名应为 commit ID。
    """
    version_file = os.path.join(version_path, VERSION_ID_FILE_NAME)
    commit_id = os.path.basename(os.path.normpath(version_path))

    with open(version_file, 'w', encoding='utf-8') as f:
        f.write(commit_id)

def run_priority_upgrade(repo_obj, trans):
    new_version_id = trans.new_revision
    if not _should_upgrade(new_version_id):
        return False

    old_version_id,old_version_config = get_current_version()

    # 创建/opt/update-module/versions/下面根目录
    version_root = os.path.join(VERSIONS_DIR, new_version_id)
    os.makedirs(version_root, exist_ok=True)

    # 下载优先升级版本内容
    version_config = add_new_version(repo_obj, trans,version_root)
    if version_config is None:
        # 优先升级失败时 删除掉下载的内容
        shutil.rmtree(version_root, ignore_errors=True)
        return False

    # 进行规则映射，将配置转移到/opt规则下面
    remapped_files = get_remapped_files(version_config.manifests)
    has_moved = handle_move_operations(remapped_files,version_root)
    version_config.moved_files.update(has_moved)
    
    # 处理需要复制文件到指定的位置
    move_files = get_move_files_from_manifests(version_config.manifests)
    has_moved = handle_move_operations(move_files,version_root)
    version_config.moved_files.update(has_moved)

    # 增加记录版本id的文件，方便进行解析
    _add_version_id_file(version_root)

    # 最后进行激活目录，保证原子性，激活之后 所有的代码被移动到/opt/update-module/current目录下面
    _activate_new_version(version_root)

    # 删除旧版本与清除旧配置
    _clean_old_versions(new_version_id, old_version_id)
    _clean_old_config(version_config.moved_files, old_version_config.moved_files)

    # 执行自更新后的服务
    post_hooks = get_all_post_hooks(version_config.manifests)
    for action in post_hooks:
        if MANIFESTS_DIR in action.command:
            action.command = os.path.join(CURRENT_PROGRESS, action.command.lstrip('/'))
    version_config.executed_hooks = execute_hooks(post_hooks)

    # 导出配置到本地
    manifest_path = os.path.join(CURRENT_PROGRESS, VERSION_MANIFEST_FILE)
    with open(manifest_path, "w") as f:
        json.dump(json.loads(version_config.json()), f, indent=4)

    return True