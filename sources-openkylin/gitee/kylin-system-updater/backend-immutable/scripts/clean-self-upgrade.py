#!/usr/bin/python3
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

UPDATE_ROOT = "/opt/update-module"
VERSIONS_DIR = os.path.join(UPDATE_ROOT, "versions")
CURRENT_PROGRESS = os.path.join(UPDATE_ROOT, "current")

VERSION_MANIFEST_FILE = "version-manifests.json"

if os.getuid() != 0:
    print("需要root权限执行脚本")
    exit(1)

version_manifest_path = os.path.join(CURRENT_PROGRESS, VERSION_MANIFEST_FILE)
if os.path.exists(version_manifest_path):
    try:
        with open(version_manifest_path, 'r') as f:
            version_config = json.load(f)
    except (OSError, json.JSONDecodeError) as e:
        print(f"Failed to load version manifest: {e}")
        exit(0)
else:
    print(f"不存在文件:{version_manifest_path}")
    exit(0)

backup_root = os.path.join(CURRENT_PROGRESS, "backup")

for dsts in version_config.get("moved_files",{}).values():
    if dsts.startswith("/opt"):
        print(f"删除文件 {dsts}")
        os.remove(dsts)
        continue

    has_file = os.path.join(backup_root, dsts.lstrip("/"))
    if os.path.exists(has_file):
        shutil.copy2(has_file, dsts)
        print(f"还原文件{has_file} -> {dsts}")
    else:
        print(f"删除文件 {dsts}")
        os.remove(dsts)

print(f"自更新的所有内容 {UPDATE_ROOT}")
shutil.rmtree(UPDATE_ROOT, ignore_errors=True)