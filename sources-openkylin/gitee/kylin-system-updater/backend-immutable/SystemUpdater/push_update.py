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

import logging
import dbus
from dataclasses import dataclass
from .Core.errors import *
from .Core.enums import *
from SystemUpdater.constants import PUTSH_CONTENT_LOCATION,DEFAULT_REPO_NAME
from gettext import gettext as _

@dataclass
class UpdateContent:
    repo_name: str = DEFAULT_REPO_NAME
    branch: str = None
    revision: str = None

#更新important.list的本次升级的列表
def update_important(bus,reply_handler,error_handler):
    def _update_important_reply(retval):
        if bool(retval) == False:
            error_handler(ROLE_UPDATE_CACHE,ERROR_UPDATE_SOURCE_FAILED)
        else:
            push_content = get_push_content()
            reply_handler(push_content)
    def _update_important_error(retval):
        logging.error(str(retval))
        error_handler(ROLE_UPDATE_CACHE,ERROR_UPDATE_SOURCE_FAILED)
    
    try:
        obj = bus.get_object('com.kylin.software.properties', '/com/kylin/software/properties')

        interface = dbus.Interface(obj, dbus_interface='com.kylin.software.properties.interface')
        interface.updateSourceTemplate(timeout=20,reply_handler=_update_important_reply,error_handler=_update_important_error)
    except Exception as e:
        logging.warning(str(e))
        raise UpdateBaseError(ERROR_UPDATE_SOURCE_FAILED)

def _read_important_data():
    # 当存在源管理推送内容时 优先从源管理获取当前推送的分支信息 
    try:
        push_list = []
        with open(PUTSH_CONTENT_LOCATION, 'r') as f:
            data = f.read()
            push_list = data.split()

        #去除重复内容
        push_list = list(set(push_list))

        logging.info("Server push list: %r",push_list)
        return push_list
    except Exception as e:
        logging.warning(str(e))
        return []

def _parse_push_content(push_content):
    # 推送的内容格式:
    # 1、带有commit号时拉取分支指定commit，仓库名称:分支名称@commit号 示例：kylin:kylin/desktop/v11/2503/xc/general/all/beta2/amd64@e46fb4558c5e0d2724af
    # 2、不带有commit号时拉取分支最新commit，kylin:kylin/2203/230  示例：kylin:kylin/desktop/v11/2503/xc/general/all/beta2/amd64
    update_content = UpdateContent()
    try:
        refspec = push_content.split(":")
        if len(refspec) == 2:
            update_content.repo_name = refspec[0]
            ref_and_rev = refspec[1]
            if "@" in ref_and_rev:
                refs_details = ref_and_rev.split("@")
                update_content.branch = refs_details[0]
                update_content.revision = refs_details[1]
            else:
                update_content.branch = ref_and_rev
            return [update_content]
        else:
            logging.error("Push updates do not contain the repo name...")
            return []
    except Exception as e:
        logging.warning(str(e))
        return []

def get_push_content():
    update_list = []
    push_list = _read_important_data()
    for cont in push_list:
        update_cont = _parse_push_content(cont)
        update_list = update_cont + update_list
    return update_list
