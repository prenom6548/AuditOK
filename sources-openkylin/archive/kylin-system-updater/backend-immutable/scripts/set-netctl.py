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
import ctypes
from ctypes import POINTER, c_char_p, c_int

def find_all_files(base_path):
    """Recursively collect all files under the given path."""
    all_files = []
    for root, _, files in os.walk(base_path):
        for name in files:
            all_files.append(os.path.join(root, name))
    return all_files

def strip_var_prefix(path):
    """Strip '/var' prefix from path if present."""
    return path[4:] if path.startswith("/var") else path

def call_set_label(lib_path, file_paths, pkgname):
    """Call the shared library function to set labels for files."""
    file_list_bytes = [p.encode("utf-8") for p in file_paths]
    files_array = (c_char_p * len(file_list_bytes))(*file_list_bytes)

    try:
        lib = ctypes.CDLL(lib_path)
        lib.setLabelForPkg.argtypes = [POINTER(c_char_p), c_int, c_char_p]
        lib.setLabelForPkg.restype = c_int

        result = lib.setLabelForPkg(files_array, len(file_list_bytes), pkgname.encode("utf-8"))
        return result
    except OSError as e:
        print(f"[ERROR] Failed to load shared library: {e}")
        return -1
    except Exception as e:
        print(f"[ERROR] Exception during setLabelForPkg call: {e}")
        return -2

def main():
    symlink_path = "/opt/update-module/current"
    real_path = os.path.realpath(symlink_path)

    modified_path = strip_var_prefix(real_path)
    file_list = find_all_files(modified_path)
    print(f"Set kysec netctl files files {len(file_list)}")

    call_set_label("/usr/lib/libksaflabelmgr.so", file_list, "kylin-system-updater")

if __name__ == "__main__":
    main()
