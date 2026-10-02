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
import subprocess
import logging
from typing import List

from .manifest import (
    PostAction,
)

import subprocess
import logging

def _execute_command(hook: str):
    """
    执行 shell 命令 hook。
    - 返回 (True, None) 表示执行成功且无错误输出。
    - 返回 (False, stderr) 表示执行失败或有错误输出。
    """
    if not hook:
        logging.info("No hook to execute.")
        return True, None  # 空钩子不视为错误

    try:
        logging.info(f"Executing hook: {hook}")
        result = subprocess.run(hook, shell=True, text=True, capture_output=True,timeout=10)

        if result.stdout:
            logging.info(f"Stdout: {result.stdout.strip()}")

        stderr = result.stderr.strip() if result.stderr else None
        if stderr:
            logging.warning(f"Stderr: {stderr}")

        if result.returncode == 0 and not stderr:
            return True, None
        else:
            return False, stderr or "Non-zero return code without stderr."

    except Exception as ex:
        logging.error(f"Failed to execute hook '{hook}': {str(ex)}")
        return False, str(ex)

def execute_hooks(actions: List[PostAction]):
    if not actions:
        logging.info("No hooks to execute.")
        return actions

    for action in actions:
        command = f"{action.command} {action.value}".strip()
        action.success,action.error_desc = _execute_command(command)

    return actions

def execute_netctl(files: List[str]):
    if not files:
        logging.info("No kysec netctl files to execute.")
        return

    for fi in files:
        if not os.path.isfile(fi):
            logging.warning(f"Skipped non-existent file: {fi}")
            continue

        command = f"/usr/sbin/kysec_netctl --set -a {fi} -s allow"
        _execute_command(command)