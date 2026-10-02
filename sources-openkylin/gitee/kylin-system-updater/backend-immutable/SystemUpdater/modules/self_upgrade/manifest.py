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
import glob
import json
import logging
import fnmatch
import filecmp
import shutil
from typing import List, Dict, Set, Optional,Union,Tuple
from pydantic import BaseModel
from optparse import OptionParser
from collections import defaultdict
from SystemUpdater.constants import UPDATER_PACKAGE_NAME
from .config import MANIFESTS_DIR
from .dpkg import classify_list_paths,DPKG_INFO_DIR,get_all_list_files

ARCH = os.uname().machine
DPKG_LIST_NOT_ARCH = "%s.list"
DPKG_LIST_ARCH = "%s:%s.list"
DPKG_LIST_NAME = "/var/lib/dpkg/info/%s"
CHECKOUT_FAILURE_RATE = 0.0

class ExtraFiles(BaseModel):
    include: List[str] = []
    exclude: List[str] = []
    move: Dict[str, Union[str, List[str]]] = {}

class PostAction(BaseModel):
    command: Optional[str] = None
    value: Optional[str] = ""
    priority: int = 100
    success: bool = True
    error_desc: str = None

class ComponentManifest(BaseModel):
    package: str
    use_dpkg_list: bool = True
    dpkg_arch_suffix: bool = False
    use_path_remap: bool = False
    exclude_patterns: List[str] = []
    path_remap: Dict[str, set] = {}
    extra_files: ExtraFiles = ExtraFiles()
    post_hooks: List[PostAction] = []
    excluded_files: List[str] = []
    installed_files: List[str] = []
    netctl_paths: List[str] = []
    manifest_file: Optional[str] = None
    list_name: Optional[str] = None
    failure_rate: float = CHECKOUT_FAILURE_RATE

def match_patterns(file_list: Set[str], patterns: List[str]) -> Set[str]:
    return {file for pattern in patterns for file in file_list if fnmatch.fnmatch(file, pattern)}

def process_extra_files(manifest: ComponentManifest, generic_exclude: List[str], base_files: Set[str]) -> Tuple[List[str], List[str]]:
    """
    根据 manifest 配置处理额外文件。

    :param manifest: 组件的 manifest 对象
    :param base_files: 基础文件集合
    :return: (最终保留的文件列表, 被排除的文件列表)
    """
    include_files = set(manifest.extra_files.include)
    all_files = base_files.union(include_files)

    # 去重
    excluded_patterns = list(set(manifest.extra_files.exclude + generic_exclude))
    excluded_files = set(match_patterns(all_files, excluded_patterns))
    final_files = all_files - excluded_files

    return list(final_files), list(excluded_files)


def parse_manifests(root_path: str) -> Dict[str, ComponentManifest]:

    default_manifest_dir = os.path.join(root_path, MANIFESTS_DIR.lstrip('/'))
    fallback_manifest_dir = "/etc/kylin-system-updater/manifests"
    manifest_dirs = [default_manifest_dir, fallback_manifest_dir]

    all_manifests: Dict[str, ComponentManifest] = {}
    list_files = get_all_list_files()

    for manifest_path in manifest_dirs:
        if not os.path.isdir(manifest_path):
            continue

        for current_root, _, filenames in os.walk(manifest_path):
            for name in filenames:
                if name.endswith(".json"):
                    full_path = os.path.join(current_root, name)
                    try:
                        with open(full_path, "r") as file:
                            data = json.load(file)
                            manifest = ComponentManifest(**data)
                            manifest.list_name = _get_use_dpkg_list_packages(list_files,manifest)
                            manifest.manifest_file = full_path
                            all_manifests[manifest.package] = manifest
                            
                            logging.info("Parsed manifest: %s", full_path)

                    except Exception as exc:
                        logging.warning("Failed to parse manifest %s: %s", full_path, exc)

    return all_manifests

def _get_use_dpkg_list_packages(list_files: List[str],manifest: ComponentManifest) -> List[str]:
    """
    根据 manifest 标志选择是否带架构后缀生成 list 文件名。
    优先使用存在的 foo.list 或 foo:arch.list，如果都不存在，
    根据 dpkg_arch_suffix 决定默认返回哪个形式的 list 名。

    同时将对应 list 文件路径写入 manifest.list_file。
    """
    
    dpkg_list_names = None

    # 架构映射
    arch_remap = {
        "aarch64": "arm64",
        "x86_64": "amd64",
    }
    system_arch = arch_remap.get(ARCH, ARCH)

    if not manifest.use_dpkg_list:
        return dpkg_list_names

    plain_name = DPKG_LIST_NOT_ARCH % manifest.package
    arch_name = DPKG_LIST_ARCH % (manifest.package, system_arch)

    if plain_name in list_files:
        dpkg_list_name = plain_name
    elif arch_name in list_files:
        dpkg_list_name = arch_name
    else:
        # 根据标志决定默认回退
        dpkg_list_name = arch_name if manifest.dpkg_arch_suffix else plain_name

    return dpkg_list_name

def get_move_files_from_manifests(manifests: Dict[str, ComponentManifest]) -> Dict[str, List[str]]:
    all_files: Dict[str, List[str]] = {}

    for manifest in manifests.values():
        for from_file, to_path in manifest.extra_files.move.items():
            if isinstance(to_path, str):
                to_paths = [to_path]
            elif isinstance(to_path, list):
                to_paths = to_path
            else:
                logging.warning(f"Invalid move target type for '{from_file}': {to_path!r}")

            # 拼接目标路径
            resolved_paths = [
                os.path.join(path, os.path.basename(from_file)) for path in to_paths
            ]
            all_files[from_file] = resolved_paths

    return all_files

def get_all_post_hooks(manifests: List[ComponentManifest]) -> List[PostAction]:
    all_actions = []
    for manifest in manifests.values():
        if manifest.post_hooks:
            all_actions.extend(manifest.post_hooks)
    return sorted(all_actions, key=lambda action: action.priority)

def _get_all_exclude_files(manifests: Dict[str, ComponentManifest]) -> List[str]:
    all_exclude_files = []
    for manifest in manifests.values():
        if manifest.exclude_patterns:
            all_exclude_files.extend(manifest.exclude_patterns)
    return all_exclude_files

def get_all_netctl_files(root_path: str,manifests: Dict[str, ComponentManifest]) -> List[str]:
    """
    收集所有 manifest 中 netctl_paths 字段定义的文件路径，使用 glob 递归匹配。
    返回去重后的文件列表（不包含目录）。
    """
    all_netctl_files = set()

    for manifest in manifests.values():
        for pattern in manifest.netctl_paths:
            abs_pattern = os.path.join(root_path, pattern.lstrip('/'))
            matched_files = glob.glob(abs_pattern, recursive=True)
            all_netctl_files.update(
                path for path in matched_files if os.path.isfile(path)  # 排除目录
            )
    
    return sorted(all_netctl_files)

def get_all_dpkg_list_files(manifests: Dict[str, ComponentManifest]) -> List[str]:
    """
    从给定的 manifests 列表中提取有效的 list_name，并构造完整路径。
    仅返回存在于 base_dir 下的文件路径。

    :param manifests: 包含 list_name 属性的对象列表（如 ComponentManifest 实例）。
    :param base_dir: dpkg list 文件所在的基础目录，默认为 /var/lib/dpkg。
    :return: 存在的 dpkg list 文件的完整路径列表。
    """
    dpkg_files = []

    for manifest in manifests.values():
        list_name = manifest.list_name
        if not list_name:
            continue

        dpkg_files.append(os.path.join(DPKG_INFO_DIR, list_name))
    logging.info("Resolved dpkg list files: %s", dpkg_files)
    return dpkg_files

def collect_installed_files_from_manifests(root_path: str,manifests: Dict[str, ComponentManifest]) -> List[str]:
    all_files: List[str] = []

    generic_exclude_patterns = _get_all_exclude_files(manifests)
    # 遍历所有的dpkg list + extra_files + 排除文件 获取所有需要拉取的文件
    for manifest in manifests.values():
        if manifest.use_dpkg_list and manifest.list_name:
            base_files = classify_list_paths(root_path,manifest.list_name)
            # 获取需要拉取的文件以及记录需要排除的文件
            manifest.installed_files,manifest.excluded_files = process_extra_files(manifest, generic_exclude_patterns, set(base_files))
        else:
            manifest.installed_files = manifest.extra_files.include

        all_files.extend(manifest.installed_files)

    return all_files


def handle_move_operations(move_map: Dict[str, Union[str, List[str]]], version_root: str) -> Dict[str, str]:
    """
    执行 move 操作，将源文件移动到一个或多个目标路径，并在移动前进行备份。
    - 目标目录不存在时自动创建；
    - 根据目标是否以 '/' 结尾来推测是目录或文件；
    - 文件存在则比对内容，相同跳过；
    - 文件不存在直接复制。
    """
    backup_root = os.path.join(version_root, "backup")
    os.makedirs(backup_root, exist_ok=True)

    moved_files: Dict[str, str] = {}

    for src, dsts in move_map.items():
        full_src = os.path.join(version_root, src.lstrip('/'))
        if not os.path.exists(full_src):
            logging.warning(f"Source file not found, skipping: {full_src}")
            continue

        dst_list = [dsts] if isinstance(dsts, str) else dsts

        for dst in dst_list:
            try:
                original_dst = dst  # 保存原始目标，方便日志

                # 推测：如果目标以 '/' 结尾，或已经存在且是目录
                if dst.endswith('/') or (os.path.exists(dst) and os.path.isdir(dst)):
                    dst = os.path.join(dst, os.path.basename(full_src))

                dst_dir = os.path.dirname(dst)

                if not os.path.isdir(dst_dir):
                    os.makedirs(dst_dir, exist_ok=True)
                    logging.info(f"Created destination directory: {dst_dir}")

                # 如果目标文件存在，进行内容比对
                if os.path.exists(dst):
                    if filecmp.cmp(full_src, dst, shallow=False):
                        logging.info(f"File unchanged, skipping: {full_src} -> {dst}")
                        moved_files[src] = dst
                        continue

                    # 文件不同，备份后覆盖
                    relative_dst_path = dst.lstrip('/')
                    backup_path = os.path.join(backup_root, relative_dst_path)
                    os.makedirs(os.path.dirname(backup_path), exist_ok=True)
                    shutil.copy2(dst, backup_path)
                    logging.info(f"Backed up: {dst} -> {backup_path}")

                # 执行复制
                shutil.copy2(full_src, dst)
                logging.info(f"Copied file: {full_src} -> {dst}")

                # 记录迁移
                moved_files[src] = dst

            except Exception as e:
                logging.error(f"Error handling move for {full_src} -> {original_dst}: {e}")

    return moved_files

def get_path_remap(manifests: Dict[str, ComponentManifest]) -> Dict[str, Set[str]]:
    combined_remapping = defaultdict(set)
    
    for pkg, manifest in manifests.items():
        for src_path, targets in manifest.path_remap.items():
            norm_src = os.path.normpath(src_path)
            norm_targets = {os.path.normpath(target) for target in targets}
            combined_remapping[norm_src].update(norm_targets)

    return dict(combined_remapping)

def get_failure_rate(manifests: Dict[str, ComponentManifest]) -> float: 
    component = manifests.get(UPDATER_PACKAGE_NAME)
    if isinstance(component, ComponentManifest) and hasattr(component, 'failure_rate'):
        return component.failure_rate
    return CHECKOUT_FAILURE_RATE

def get_remapped_files(manifests: Dict[str, ComponentManifest]) -> Dict[str, List[str]]:
    remapped: Dict[str,List[str]] = {}
    files = []

    mapping = get_path_remap(manifests)

    for pkg, manifest in manifests.items():
        if manifest.use_path_remap:
            files = files + manifest.installed_files

    for f in files:
        for prefix, targets in mapping.items():
            normalized_prefix = prefix.rstrip("/") + "/"
            if f.startswith(normalized_prefix):
                relative_path = f[len(normalized_prefix):]
                target_paths = []
                for target_base in targets:
                    target_base = target_base.rstrip("/") + "/"
                    target_paths.append(target_base + relative_path)
                remapped[f] = target_paths
                break  # 假设一个文件只匹配一个规则前缀

    return remapped

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-f", "--files", action="store", type="string", dest="files", help="Specify input files for parsing")
    (options, args) = parser.parse_args()