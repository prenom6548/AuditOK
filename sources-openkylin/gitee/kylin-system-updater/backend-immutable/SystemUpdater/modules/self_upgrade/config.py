# config.py
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

UPDATE_ROOT = "/opt/update-module"
VERSIONS_DIR = os.path.join(UPDATE_ROOT, "versions")
CURRENT_PROGRESS = os.path.join(UPDATE_ROOT, "current")
MANIFESTS_DIR = "/usr/share/update-manifests"

BACKUP_ROOT_DIR = "/var/cache/kylin-system-updater/backup"
VERSION_MANIFEST_FILE = "version-manifests.json"