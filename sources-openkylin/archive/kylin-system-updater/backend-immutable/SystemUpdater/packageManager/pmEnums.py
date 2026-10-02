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

import gettext
gettext.bindtextdomain('kylin-system-updater', '/usr/share/locale')
gettext.textdomain('kylin-system-updater')
_ = gettext.gettext

DEFAULT_EMPTY_CHATACTER = ''

ERROR_READ_LOCAL_DEB = "error-read-local-deb"
ERROR_LOCAL_DEB_FORMAT = "error-local-deb-format"
ERROR_INSTALL_DEB_BASE = "error-install-deb-base"
ERROR_INSTALL_DEB_VERIFY = "error-install-deb-verify"
ERROR_INSTALL_DEB_SECURITY = "error-install-deb-security"
ERROR_INSTALL_DEB_DPKG = "error-install-deb-dpkg"
ERROR_INSTALL_DEB_DEPEND = "error-install-deb-depend"
ERROR_INSTALL_DEB_OTHER = "error-install-deb-other"
ERROR_INSTALL_TIMEOUT_FAILED = "error-install-timeout-failed"

ERROR_PURGE_PKG_DPKG = "error-purge-pkg-dpkg"
ERROR_PURGE_PKG_VERIFY = "error-purge-pkg-verify"
ERROR_PURGE_PKG_TOREMOVE = "error-purge-pkg-toremove"


_STRINGS_ERROR = {
    DEFAULT_EMPTY_CHATACTER: '',

    ERROR_READ_LOCAL_DEB:_(" "),
    ERROR_LOCAL_DEB_FORMAT:_(" "),
    ERROR_INSTALL_DEB_BASE:_(" "),
    ERROR_INSTALL_DEB_VERIFY:_(" "),
    ERROR_INSTALL_DEB_SECURITY:_(" "),
    ERROR_INSTALL_DEB_DPKG:_(" "),
    ERROR_INSTALL_DEB_DEPEND:_(" "),
    ERROR_INSTALL_DEB_OTHER:_(" "),
    ERROR_INSTALL_TIMEOUT_FAILED: _(" "),

    ERROR_PURGE_PKG_DPKG:_(" "),
    ERROR_PURGE_PKG_VERIFY:_(" "),
    ERROR_PURGE_PKG_TOREMOVE:_(" ")
}

_DESCS_ERROR = {

    DEFAULT_EMPTY_CHATACTER: '',

    ERROR_READ_LOCAL_DEB:_("Deb format exception, read local deb file error."),
    ERROR_LOCAL_DEB_FORMAT:_("Deb format exception, failed to parse package file."),
    ERROR_INSTALL_DEB_BASE:_("Install deb error."),
    ERROR_INSTALL_DEB_VERIFY:_(" "),
    ERROR_INSTALL_DEB_SECURITY:_(" "),
    ERROR_INSTALL_DEB_DPKG:_(" "),
    ERROR_INSTALL_DEB_DEPEND:_(" "),
    ERROR_INSTALL_DEB_OTHER:_(" "),
    ERROR_INSTALL_TIMEOUT_FAILED: _("Installation timeout and the installation is exited."),

    ERROR_PURGE_PKG_DPKG:_(" "),
    ERROR_PURGE_PKG_VERIFY:_(" "),
    ERROR_PURGE_PKG_TOREMOVE:_(" ")
}

_CODE_ERROR = {
    DEFAULT_EMPTY_CHATACTER: '',

    ERROR_READ_LOCAL_DEB: "#0205",
    ERROR_LOCAL_DEB_FORMAT: "#0206",
    ERROR_INSTALL_DEB_BASE: "#0207",
    ERROR_INSTALL_DEB_VERIFY:"#0211",
    ERROR_INSTALL_DEB_SECURITY:"#0212",
    ERROR_INSTALL_DEB_DPKG:"#0213",
    ERROR_INSTALL_DEB_DEPEND:"#0214",
    ERROR_INSTALL_DEB_OTHER:"#0220",
    ERROR_INSTALL_TIMEOUT_FAILED: "#0218",

    ERROR_PURGE_PKG_DPKG:"#0221",
    ERROR_PURGE_PKG_VERIFY:"#0222",
    ERROR_PURGE_PKG_TOREMOVE:"#0223",
}


#UPGRADE MONITOR STATUS
MONIT_DETECT = "step-updatedetect"
MONIT_DEPRESOLUT = "step-depresolution"
MONIT_DOWNLOAD = "step-downfinish"
MONIT_INSTALL = "step-installing"
MONIT_FINISH = "step-finish"
MONIT_INSTALLDEB = "step-installdeb"

SOURCE_NAME = {
    'kylin-installer':_("Kylin Installer"),
    'kylin-uninstaller':_("Kylin Uninstaller"),
    'kylin-background-upgrade':_("Kylin Background Upgrade"),
    'kylin-software-center':_("Kylin Software Center"),
}

CALLER = {
    'kylin-installer':"Kylin Installer",
    'kylin-uninstaller':"Kylin Uninstaller",
    'kylin-background-upgrade':"Kylin Background Upgrade",
    'kylin-software-center':"Kylin Software Center",
}

PolicyKit_Authority_Action = {
    'kylin-installer':"cn.kylin.installer.action",
    'kylin-uninstaller':"cn.kylin.uninstaller.action",
    'kylin-software-center':"cn.kylin.software.center.action",
    'kylin-system-updater':"cn.kylinos.KylinSystemUpdater.action",
    'kylin-installer-self':"cn.kylin.installer.self.action",
    'kylin-uninstaller-self':"cn.kylin.uninstaller.self.action",
    'kylin-software-center-self':"cn.kylin.software.center.self.action",
    'kylin-system-updater-self':"cn.kylinos.KylinSystemUpdater.self.action",
}


def get_source_name_from_enum(enum):
    try:
        return _(SOURCE_NAME[enum])
    except KeyError:
        return _("Kylin System Updater")

def get_caller_from_enum(enum):
    try:
        return CALLER[enum]
    except KeyError:
        return _("Kylin System Updater")

def get_policykit_authority_action_enum(enum):
    try:
        return PolicyKit_Authority_Action[enum]
    except KeyError:
        if enum.endswith("-self"):
            return "cn.kylinos.KylinSystemUpdater.self.action" #默认配置
        else:
            return "cn.kylinos.KylinSystemUpdater.action" #默认配置

# vim:ts=4:sw=4:et
