#!/usr/bin/env python3
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

import sys
import hashlib
import os
import json
import shutil
import subprocess
import logging
from pathlib import Path
from typing import Generator, Tuple, Optional, List, Dict, Any

# 日志配置
logging.basicConfig(
    format='[%(levelname)s] %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)

# 配置项
CONFIG = {
    "UPDATE_ROOT": Path("/opt/update-module"),
    "VERSIONS_DIR": None,  # 稍后赋值
    "CURRENT_PROGRESS": None,
    "MANIFESTS_DIR": Path("/usr/share/update-manifests"),
    "BACKUP_ROOT_DIR": Path("/var/cache/kylin-system-updater/backup"),
    "VERSION_MANIFEST_FILE": "version-manifests.json",
}

CONFIG["VERSIONS_DIR"] = CONFIG["UPDATE_ROOT"] / "versions"
CONFIG["CURRENT_PROGRESS"] = CONFIG["UPDATE_ROOT"] / "current"


def check_args() -> List[Path]:
    if len(sys.argv) < 2:
        logger.error(f"用法: {sys.argv[0]} <包1.deb> [包2.deb ...]")
        sys.exit(1)
    paths = [Path(p) for p in sys.argv[1:]]
    logger.debug(f"输入包列表: {paths}")
    return paths


def generate_version_path(base_dir: Path) -> Path:
    random_hash = hashlib.sha256(os.urandom(32)).hexdigest()
    version_path = base_dir / random_hash
    try:
        version_path.mkdir(parents=True, exist_ok=False)
        logger.info(f"创建新版本目录: {version_path}")
    except FileExistsError:
        logger.warning(f"版本目录已存在，尝试重新生成路径")
        return generate_version_path(base_dir)  # 递归尝试
    return version_path


def extract_deb_packages(packages: List[Path], target_dir: Path) -> None:
    for pkg in packages:
        if not pkg.is_file():
            logger.warning(f"文件不存在，跳过解压: {pkg}")
            continue
        logger.info(f"解压包: {pkg} → {target_dir}")
        try:
            subprocess.run(["dpkg", "-x", str(pkg), str(target_dir)], check=True)
            logger.info(f"解压成功: {pkg}")
        except subprocess.CalledProcessError as e:
            logger.error(f"解压失败: {pkg}，错误: {e}")


def load_manifests(manifest_dir: Path) -> Generator[Tuple[Path, Dict[str, Any]], None, None]:
    if not manifest_dir.exists():
        logger.warning(f"Manifest目录不存在: {manifest_dir}")
        return
    for file in manifest_dir.rglob("*.json"):
        try:
            with file.open("r", encoding="utf-8") as f:
                manifest_data = json.load(f)
            logger.info(f"加载Manifest文件: {file}")
            yield file, manifest_data
        except Exception as e:
            logger.error(f"解析Manifest失败: {file}，错误: {e}")


def process_moves(moves: Dict[str, Any]) -> None:
    for src_str, dests in moves.items():
        src = Path(src_str)
        if not src.exists():
            logger.warning(f"移动源文件不存在，跳过: {src}")
            continue
        dest_list = dests if isinstance(dests, list) else [dests]
        for dest in dest_list:
            dest_path = Path(dest)
            try:
                dest_path.parent.mkdir(parents=True, exist_ok=True)
                if src.is_dir():
                    if dest_path.exists():
                        shutil.rmtree(dest_path)
                    shutil.copytree(src, dest_path)
                else:
                    shutil.copy2(src, dest_path)
                logger.info(f"移动成功: {src} → {dest_path}")
            except Exception as e:
                logger.error(f"移动失败: {src} → {dest_path}，错误: {e}")


def execute_command(hook: str) -> Tuple[bool, Optional[str]]:
    hook = hook.strip()
    if not hook:
        logger.debug("空钩子，跳过执行。")
        return True, None
    logger.info(f"执行钩子命令: {hook}")
    try:
        result = subprocess.run(hook, shell=True, text=True, capture_output=True)
        stdout = result.stdout.strip() if result.stdout else None
        stderr = result.stderr.strip() if result.stderr else None
        if stdout:
            logger.info(f"钩子输出: {stdout}")
        if stderr:
            logger.warning(f"钩子错误输出: {stderr}")
        success = (result.returncode == 0) and (not stderr)
        return success, stderr if not success else None
    except Exception as ex:
        logger.error(f"执行钩子失败: '{hook}'，错误: {ex}")
        return False, str(ex)


def run_post_hooks(hooks: List[Dict[str, Any]], version_path: Path) -> None:
    sorted_hooks = sorted(hooks, key=lambda x: x.get("priority", 0))
    for hook in sorted_hooks:
        cmd = hook.get("command")
        val = hook.get("value")
        if not cmd:
            logger.debug("跳过无命令钩子。")
            continue
        if val == "/opt/update-module/current":
            val = str(version_path)
        full_cmd = f"{cmd} {val}" if val else cmd
        success, err = execute_command(full_cmd)
        if not success:
            logger.error(f"钩子执行失败: {full_cmd}，错误信息: {err}")


def update_current_symlink(CURRENT_PROGRESS: Path, target_path: Path) -> None:
    try:
        if CURRENT_PROGRESS.exists() or CURRENT_PROGRESS.is_symlink():
            CURRENT_PROGRESS.unlink()
        CURRENT_PROGRESS.symlink_to(target_path)
        logger.info(f"软链接已更新: {CURRENT_PROGRESS} → {target_path}")
    except Exception as e:
        logger.error(f"更新软链接失败: {e}")


def main() -> None:
    packages = check_args()
    version_path = generate_version_path(CONFIG["VERSIONS_DIR"])

    extract_deb_packages(packages, version_path)

    # 处理所有 manifest 文件
    for manifest_path, manifest in load_manifests(CONFIG["MANIFESTS_DIR"]):
        logger.info(f"开始处理 Manifest 文件: {manifest_path}")
        extra_files = manifest.get("extra_files", {})
        moves = extra_files.get("move")
        if moves:
            process_moves(moves)
        post_hooks = manifest.get("post_hooks")
        if post_hooks:
            run_post_hooks(post_hooks, version_path)

    # 更新软链接指向新版本
    update_current_symlink(CONFIG["CURRENT_PROGRESS"], version_path)

    logger.info("✅ 所有操作成功完成。")


if __name__ == "__main__":
    main()
