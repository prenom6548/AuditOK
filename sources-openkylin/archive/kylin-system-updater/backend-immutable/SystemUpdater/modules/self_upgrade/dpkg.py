# utils.py
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
from optparse import OptionParser
from typing import List, Dict

# 从dpkg list中统一移除那些文件特殊的标识
REMOVE_FILES_LIST = ["/."]
DPKG_INFO_DIR = "/var/lib/dpkg/info"

def get_all_list_files():
    return {f for f in os.listdir(DPKG_INFO_DIR) if f.endswith('.list')}

def classify_list_paths(root_path,list_name):
    list_full_path = os.path.join(DPKG_INFO_DIR, list_name)
    list_actual_path = os.path.join(root_path, list_full_path.lstrip('/'))

    if not os.path.exists(list_actual_path):
        logging.info("dpkg list file is not exists: %s", list_actual_path)
        return []
    
    with open(list_actual_path, 'r') as f:
        paths = [line.strip() for line in f if line.strip()]
    all_paths = set(paths)
    dir_paths = set()

    for path in all_paths:
        for other in all_paths:
            if other != path and other.startswith(path.rstrip('/') + '/'):
                dir_paths.add(path)
                break
    file_paths = all_paths - dir_paths

    for file in REMOVE_FILES_LIST:
        file_paths.remove(file)

    return sorted(file_paths)

def get_versions_from_status(packages: List[str], status_file: str) -> Dict[str, str]:
    versions = {}
    current_block = ""
    found_packages = set(packages)

    if not os.path.exists(status_file):
        logging.info("Status file is not exists: %s", status_file)
        return versions
    
    with open(status_file, "r") as f:
        for line in f:
            if line.strip() == "":
                pkg_name = None
                version = None
                for entry in current_block.splitlines():
                    if entry.startswith("Package: "):
                        pkg_name = entry.split(":", 1)[1].strip()
                    elif entry.startswith("Version: "):
                        version = entry.split(":", 1)[1].strip()
                if pkg_name in found_packages:
                    versions[pkg_name] = version
                    found_packages.remove(pkg_name)
                    if not found_packages:
                        break
                current_block = ""
            else:
                current_block += line
    return versions


if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option("-f", "--files",
                        action="store", type="string", dest="files",
                        help="输入需要解析的文件")
    (options, args) = parser.parse_args()

    file_paths = classify_list_paths(options.files)

    for file in file_paths:
        print(file)
