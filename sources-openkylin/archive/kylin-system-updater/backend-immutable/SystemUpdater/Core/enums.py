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

#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""enums - Enumerates for apt daemon dbus messages"""

# __all__ = (
#             #错误枚举
#             "ERROR_OTHER_PROGRESS_OCCUPATION","ERROR_NETWORK_FAILED","EXIT_CANCELLED","ERROR_NOT_ROLLBAK_DEPLOYMENT",
#             "ERROR_ORIGIN_IS_NONE","INSTALL_FAILED_FALG","INSTALL_SUCCESS_FLAG","INSTALL_DEFAULT_FLAG","ERROR_UPDATE_SOURCE_FAILED",
#             "ERROR_PROGRAM_EXCEPTION","ERROR_NOT_DISK_SPACE","ERROR_PUSH_BRANCH_EXCEPTION","ERROR_REPO_IS_NONE",

#             # 方法
#             "get_error_description_from_enum","get_error_num_from_enum"
#         )

import gettext
gettext.bindtextdomain('kylin-system-updater', '/usr/share/locale')
gettext.textdomain('kylin-system-updater')
_ = gettext.gettext



# TRANSACTION STATES
#: The transaction was created, but hasn't been queued.
STATUS_SETTING_UP = "status-setting-up"

#: 正在下载过程中
STATUS_DOWNLOADING = "status-downloading"

#: 正在部署过程中
STATUS_DEPLOYING = "status-deploying"

STATUS_FINISHED = "status-finished"

STATUS_CANCELLING = "status-cancelling"


# 更新事务进行过程中的角色
ROLE_UNSET = "role-unset"
# 检查更新
ROLE_UPDATE_CACHE = "role-update-cache"
#更新下载
ROLE_DOWNLOAD = "role-download"
# 更新部署
ROLE_DEPLOY = "role-deploy"


#系统更新的所有枚举
PRIORITY_UPGRADE_SUCCCESSED = "priority-upgrade-successed"
INSTALL_DEFAULT_FLAG = 'none'
INSTALL_FAILED_FALG = 'failed'
INSTALL_SUCCESS_FLAG = 'success'
#apt update阶段出现的错误解析

#通用错误
ERROR_PROGRAM_EXCEPTION = "error-program-exception"
ERROR_OTHER_PROGRESS_OCCUPATION = "error-other-progress-occupation"
ERROR_NETWORK_FAILED = "error-network-failed"

#检查更新错误
ERROR_ORIGIN_IS_NONE = "error-origin-is-none"
ERROR_REPO_IS_NONE = "error-repo-is-none"
ERROR_UPDATE_SOURCE_FAILED = "error-update-source-failed"
ERROR_PUSH_BRANCH_EXCEPTION = "error-push-branch-exception"


#下载错误
EXIT_CANCELLED = "exit-cancelled"
ERROR_NOT_DISK_SPACE = "error-not-disk-space"

#部署错误
ERROR_NOT_DEPLOYMENT = "error-not-deployment"
ERROR_NOT_ROLLBAK_DEPLOYMENT = "error-not-rollback-deployment"
ERROR_INSTALL_POWER_OUTAGE = "error-install-power-outage"


GLIB_ERROR_NETWORK_FAILED = "While fetching mirrorlist"

_DESCS_ERROR = {
                    ERROR_OTHER_PROGRESS_OCCUPATION: _("Other tasks are being updated and upgraded, please try again later."),
                    PRIORITY_UPGRADE_SUCCCESSED: _("Update Manager upgrade is complete, please restart the setting panel before performing the system update."),
                    ERROR_NETWORK_FAILED: _("Network anomaly, can't check for updates!"),
                    ERROR_REPO_IS_NONE: _("Name of the repository deployed by the current system is empty."),

                    ERROR_UPDATE_SOURCE_FAILED: _("Unable to access the source management server, please try again later"),
                    EXIT_CANCELLED: _("Cancel Upgrade"),
                    ERROR_NOT_ROLLBAK_DEPLOYMENT: _("There is no way to rollback to the previous version, there is nothing to rollback."),
                    ERROR_ORIGIN_IS_NONE: _("Check for update exceptions! Please verify that the system deployment is complete."),
                    ERROR_NOT_DISK_SPACE: _("Disk space is insufficient, please clean the disk and then upgrade"),
                    ERROR_PUSH_BRANCH_EXCEPTION: _("Update push is abnormal, and the corresponding branch information cannot be found on the server."),

                    # 更新部署错误
                    ERROR_NOT_DEPLOYMENT: _("The new version of the system is deployed abnormally and cannot be upgraded."),
                    ERROR_INSTALL_POWER_OUTAGE: _("The installation process is abnormally shut down or down.")
                }
_CODE_ERROR = {
    #通用错误码
    ERROR_PROGRAM_EXCEPTION: "#0000",
    ERROR_NETWORK_FAILED: "#0001",
    ERROR_OTHER_PROGRESS_OCCUPATION: "#0002",
    ERROR_PUSH_BRANCH_EXCEPTION: "#0003",
    ERROR_REPO_IS_NONE: "#0004",
    PRIORITY_UPGRADE_SUCCCESSED: "#0005",

    #检查更新错误
    ERROR_UPDATE_SOURCE_FAILED: "#0100",
    ERROR_ORIGIN_IS_NONE: "#0101",

    #下载错误 2
    EXIT_CANCELLED: "#0200",
    ERROR_NOT_DISK_SPACE: "#0201",

    #部署错误 3
    ERROR_NOT_DEPLOYMENT: "#0300",
    ERROR_NOT_ROLLBAK_DEPLOYMENT: "#0301",
    ERROR_INSTALL_POWER_OUTAGE: "#0302"
}

def get_error_num_from_enum(enum):
    """Get a long description of an error.

    :param enum: The transaction error enum, e.g. :data:`ERROR_NO_LOCK`.
    :returns: The description string.
    """
    if not enum:
        return ''
    try:
        return _CODE_ERROR[enum]
    except KeyError:
        return enum
        
def get_error_description_from_enum(enum):
    """Get a long description of an error.

    :param enum: The transaction error enum, e.g. :data:`ERROR_NO_LOCK`.
    :returns: The description string.
    """
    if not enum:
        return ''
    try:
        return _DESCS_ERROR[enum]
    except KeyError:
        return ''

# vim:ts=4:sw=4:et
