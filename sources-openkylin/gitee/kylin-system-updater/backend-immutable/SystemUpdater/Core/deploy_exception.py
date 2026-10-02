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

from SystemUpdater.constants import PUTSH_CONTENT_LOCATION,DEFAULT_REPO_NAME
from gettext import gettext as _

EXCEPTION_SHUTDOWN_FLAG = "/var/lib/kylin-system-updater/remain/install-exception-flag"


def make_execption_check_flag(self):
    if not os.path.exists(UPDATE_NOT_RESOTRE_DIR):
        os.makedirs(UPDATE_NOT_RESOTRE_DIR)

    with open(EXCEPTION_SHUTDOWN_FLAG, 'w') as file:
        pass  # 不写入任何内容，只创建文件