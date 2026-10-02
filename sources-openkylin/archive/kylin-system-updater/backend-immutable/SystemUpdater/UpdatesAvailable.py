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

#!/usr/bin/python3

import logging
from gi.repository import Gio
from gettext import gettext as _
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from dataclasses import dataclass, asdict,field
from SystemUpdater.Core.DataAcquisition import create_time_stamp

@dataclass
class DeployState:
    distro: str = None
    deployed_refs: str = None
    revision: str = None
    rollback_deployment: list = field(default_factory=list)
    timestamp: str = None
    def asdict(self):
        self.timestamp = create_time_stamp()[0:-4]
        return asdict(self)

@dataclass
class Transaction:
    success: bool = False
    sender: str = None
    from_revision: str = None
    new_revision: str = None
    status: str = None
    available_refs: str = None
    deployed_refs: str = None
    available_repo: str = None
    timestamp: str = None
    error: str = None
    push_list: list = field(default_factory=list)
    role: str = ROLE_UNSET
    update_metadata: dict = field(default_factory=dict)

    # 标识部署完成后 是重启还是关机
    reboot_required: bool = True

    # 表示前端是否已经读取当前的更新状态 读取后会将此值设置为True
    has_read: bool = False

    def cancelled():
        logging.info("Task has been cancelled...")
    cancellable = Gio.Cancellable.new()
    cancellable.connect(cancelled)

    def asdict(self):
        self.timestamp = create_time_stamp()[0:-4]
        return asdict(self)