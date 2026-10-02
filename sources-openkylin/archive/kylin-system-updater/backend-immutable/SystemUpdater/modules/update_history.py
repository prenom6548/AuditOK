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

from SystemUpdater.configs import settings
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *

def get_last_deploy_result():
    status = INSTALL_DEFAULT_FLAG
    # 新版本兼容获取更新历史
    history = settings.get("history",[])
    first_item = next(iter(history), {})
    if first_item and not first_item.get("has_read",True):
        # 前端没有读取时 返回状态
        if first_item.get("success",True):
            status = INSTALL_SUCCESS_FLAG
        else:
            status = first_item.get("error",None)
            if status is None:
                status = ""
            else:
                # 转化为错误码
                status = get_error_num_from_enum(status)
        # 标识 前端已经读取状态
        first_item["has_read"] = True
        settings.save()

    # 旧版本兼容，当history为空时，使用status
    if not first_item and settings.get("status",INSTALL_DEFAULT_FLAG) != INSTALL_DEFAULT_FLAG:
        status = settings.get("status")
    
    return status