#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""enums - Enumerates for apt daemon dbus messages"""

__all__ = (
           "ERROR_UPDATE_DEFAULT_FAILED",
           "ERROR_UPDATE_SOURCE_FAILED","ERROR_NETWORK_FAILED","ERROR_NOT_GROUPS_CONFIG","ERROR_SOFTWARE_INDEX_RROKEN",
           "ERROR_NOT_INIT_PACKAGESINFIO","ERROR_READ_IMPORTANTLIST_FAILED","ERROR_RESOLVER_FAILED","ERROR_NOT_UPGRADE_PACKAGES",
           "ERROR_REMOVE_ESSENTIAL_PACKAGES","ERROR_NOT_DISK_SPACE","ERROR_NOT_CONFIGPKG_DEPENDENCIES","ERROR_NOT_SELFPKG_DEPENDENCIES",
           "ERROR_NOT_FIX_SYSTEM","ERROR_READ_LOCAL_DEB","ERROR_LOCAL_DEB_FORMAT","ERROR_INSTALL_DEB_BASE","ERROR_LOAD_CONFIG_FAILED",
           "ERROR_UPDATE_KEY_SIGNATURES","ERROR_UPDATE_NET_AUTHENTICATION","ERROR_UPDATE_NOTREAD_SOURCES","PRIORITY_UPGRADE_SUCCCESSED",
           "ERROR_UPDATE_INVALID_TIME","ERROR_UPDATE_SOURCE_TIMEOUT","ERROR_PROGRAM_EXCEPTION","ERROR_RUN_SCRIPTS_FAILED",

           "get_error_description_from_enum", "get_error_string_from_enum", "get_source_name_from_enum", "get_caller_from_enum",
           "get_error_code_from_enum","get_policykit_authority_action_enum"
        )

import gettext
gettext.bindtextdomain('kylin-system-updater', '/usr/share/locale')
gettext.textdomain('kylin-system-updater')
_ = gettext.gettext

from aptdaemon.enums import (
                            ERROR_REPO_DOWNLOAD_FAILED
                            )

PRIORITY_UPGRADE_SUCCCESSED = "priority-upgrade-successed"

ERROR_PROGRAM_EXCEPTION = "error_program_exception"

#apt update阶段出现的错误解析
ERROR_UPDATE_DEFAULT_FAILED = "error-update-default-failed"
ERROR_UPDATE_KEY_SIGNATURES = "The following signatures"
ERROR_UPDATE_NET_AUTHENTICATION ="does the network require authentication?"
ERROR_UPDATE_NOTREAD_SOURCES = "The list of sources could not be read"
ERROR_UPDATE_INVALID_TIME = "(invalid for another"

ERROR_UPDATE_SOURCE_FAILED = "error-update-source-failed"
ERROR_UPDATE_SOURCE_TIMEOUT = "error-update-source-timeout"
ERROR_NETWORK_FAILED = "error-network-failed"
ERROR_NOT_GROUPS_CONFIG = "error-not-groups-config"
ERROR_NOT_CONFIGPKG_DEPENDENCIES = "error-not-configpkg-dependencies"
ERROR_NOT_SELFPKG_DEPENDENCIES = "error-not-selfpkg-dependencies"

ERROR_NOT_FIX_SYSTEM = "error-not-fix-system"

ERROR_LOAD_CONFIG_FAILED = "error-load-config-failed"

#自己的
ERROR_SOFTWARE_INDEX_RROKEN = "error-software-index-broken"
ERROR_NOT_INIT_PACKAGESINFIO = "error-not-init-packagesinfo"
ERROR_READ_IMPORTANTLIST_FAILED = "error-read-importantlist-failed"
ERROR_RESOLVER_FAILED = "error-resolver-failed"
ERROR_NOT_UPGRADE_PACKAGES = "error-not-upgrade-packages"
ERROR_REMOVE_ESSENTIAL_PACKAGES = "error-remove-essential-packages"
ERROR_RUN_SCRIPTS_FAILED = "error-run-scripts-failed"
ERROR_NOT_DISK_SPACE = "error-not-disk-space"
ERROR_READ_LOCAL_DEB = "error-read-local-deb"
ERROR_LOCAL_DEB_FORMAT = "error-local-deb-format"
ERROR_INSTALL_DEB_BASE = "error-install-deb-base"

_STRINGS_ERROR = {
    PRIORITY_UPGRADE_SUCCCESSED: _("Update Manager upgrade is complete, please restart the setting panel before performing the system update."),
    
    #update
    ERROR_UPDATE_DEFAULT_FAILED: _("Check for update exceptions!"),
    ERROR_UPDATE_SOURCE_FAILED: _("Check for update exceptions!"),
    ERROR_UPDATE_SOURCE_TIMEOUT: _("Check for update exceptions!"),
    ERROR_NETWORK_FAILED: _("Network anomaly, can't check for updates!"),
    ERROR_UPDATE_KEY_SIGNATURES: _("Check for update exceptions!"),
    ERROR_READ_IMPORTANTLIST_FAILED: _("Check for update exceptions!"),
    ERROR_SOFTWARE_INDEX_RROKEN: _("Check for update exceptions!"),
    ERROR_NOT_INIT_PACKAGESINFIO: _("Check for update exceptions!"),
    ERROR_NOT_FIX_SYSTEM: _("Check for update exceptions!"),
    ERROR_LOAD_CONFIG_FAILED: _("Check for update exceptions!"),
    ERROR_PROGRAM_EXCEPTION: _("Check for update exceptions!"),
    
    #优先升级
    ERROR_NOT_GROUPS_CONFIG: _("Upgrade configuration acquisition exception."),
    ERROR_NOT_CONFIGPKG_DEPENDENCIES: _("Upgrade configuration acquisition exception."),
    ERROR_NOT_SELFPKG_DEPENDENCIES: _("Priority upgrade status exception."),
    
    #install
    ERROR_RESOLVER_FAILED: _("Could not calculate the upgrade"),
    ERROR_NOT_UPGRADE_PACKAGES: _("There is an exception in the update package."),
    ERROR_REMOVE_ESSENTIAL_PACKAGES: _("There is an exception in the update package."),
    ERROR_RUN_SCRIPTS_FAILED: _("There is an exception in the update package."),
    ERROR_NOT_DISK_SPACE: _("Disk space is insufficient, please clean the disk and then upgrade"),
    ERROR_READ_LOCAL_DEB:_(" "),
    ERROR_LOCAL_DEB_FORMAT:_(" "),
    ERROR_INSTALL_DEB_BASE:_(" ")}

_DESCS_ERROR = {
    ERROR_PROGRAM_EXCEPTION: _("Program exception, please contact the administrator to solve."),
    #update 
    ERROR_UPDATE_SOURCE_FAILED: _("Unable to access the source management server, please try again later"),
    ERROR_UPDATE_SOURCE_TIMEOUT: _("Access to the source management server timed out, please try again later"),
    ERROR_NETWORK_FAILED: _("Please check your network connection and retry."),
    ERROR_UPDATE_KEY_SIGNATURES: _("Check your source public key signature"),
    ERROR_UPDATE_NOTREAD_SOURCES: _("Please check your source list and retry."),
    ERROR_UPDATE_INVALID_TIME: _("Please check the system time and synchronize the system time before updating."),
    ERROR_UPDATE_NET_AUTHENTICATION: _("Check if your network requires authentication?"),
    ERROR_NOT_GROUPS_CONFIG: _("Unable to get group configuration package, Please check if the configuration package exists in the software source repository."),
    ERROR_NOT_INIT_PACKAGESINFIO: _("An unresolvable problem occurred while initializing the package."),
    ERROR_SOFTWARE_INDEX_RROKEN: _("Software index is broken") + _("It is impossible to install or remove any software. "
                    "Please use the package manager \"Synaptic\" or run "
                    "\"sudo apt-get install -f\" in a terminal to fix "
                    "this issue at first."),
    ERROR_READ_IMPORTANTLIST_FAILED: _("read important list failed"),
    ERROR_NOT_CONFIGPKG_DEPENDENCIES: _("Unable to install group configuration package, Please check the configuration package related dependencies."),
    ERROR_NOT_SELFPKG_DEPENDENCIES: _("Unable to perform priority upgrade, please check the dependency related to the priority upgrade package."),

    ERROR_LOAD_CONFIG_FAILED: _("The system update configuration file is read abnormally, please check if the system update configuration file format is correct."),
    ERROR_NOT_FIX_SYSTEM: _("The system APT environment is abnormal, please check the system APT environment."),

    #install
    ERROR_RESOLVER_FAILED: _("nothing"),
    ERROR_NOT_UPGRADE_PACKAGES: _("This update cannot detect the upgradeable package."),
    ERROR_REMOVE_ESSENTIAL_PACKAGES: _("You request the removal of a system-essential package."),
    ERROR_RUN_SCRIPTS_FAILED:_("Exceptions to running the check script."),
    ERROR_NOT_DISK_SPACE: _("test"),
    ERROR_READ_LOCAL_DEB:_("Deb format exception, read local deb file error."),
    ERROR_LOCAL_DEB_FORMAT:_("Deb format exception, failed to parse package file."),
    ERROR_INSTALL_DEB_BASE:_("Install deb error.")
    }

_CODE_ERROR = {
    ERROR_PROGRAM_EXCEPTION: "#0000",
    #update 
    ERROR_UPDATE_SOURCE_FAILED: "#0100",
    ERROR_UPDATE_SOURCE_TIMEOUT: "#0101",
    ERROR_NETWORK_FAILED: "#0102",
    ERROR_UPDATE_KEY_SIGNATURES: "#0103",
    ERROR_UPDATE_NOTREAD_SOURCES: "#0104",
    ERROR_UPDATE_INVALID_TIME: "#0105",
    ERROR_UPDATE_NET_AUTHENTICATION: "#0106",
    ERROR_NOT_GROUPS_CONFIG: "#0107",
    ERROR_NOT_INIT_PACKAGESINFIO: "#0108",
    ERROR_SOFTWARE_INDEX_RROKEN: "#0109",
    ERROR_READ_IMPORTANTLIST_FAILED: "#0110",
    ERROR_NOT_CONFIGPKG_DEPENDENCIES: "#0111",
    ERROR_NOT_SELFPKG_DEPENDENCIES: "#0112",
    ERROR_LOAD_CONFIG_FAILED: "#0113",
    ERROR_NOT_FIX_SYSTEM: "#0114",
    ERROR_REPO_DOWNLOAD_FAILED: "#0115",

    #install
    ERROR_RESOLVER_FAILED: "#0200",
    ERROR_NOT_UPGRADE_PACKAGES: "#0201",
    ERROR_REMOVE_ESSENTIAL_PACKAGES: "#0202",
    ERROR_RUN_SCRIPTS_FAILED: "#0203",
    ERROR_NOT_DISK_SPACE: "#0204",
    ERROR_READ_LOCAL_DEB: "#0205",
    ERROR_LOCAL_DEB_FORMAT: "#0206",
    ERROR_INSTALL_DEB_BASE: "#0207"
    }


#UPGRADE MONITOR STATUS
MONIT_DETECT = "step-updatedetect"
MONIT_DEPRESOLUT = "step-depresolution"
MONIT_DOWNLOAD = "step-downloading"
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
}

def get_error_description_from_enum(enum):
    """Get a long description of an error.

    :param enum: The transaction error enum, e.g. :data:`ERROR_NO_LOCK`.
    :returns: The description string.
    """
    try:
        return _DESCS_ERROR[enum]
    except KeyError:
        return None

def get_error_code_from_enum(enum):
    """Get a long description of an error.

    :param enum: The transaction error enum, e.g. :data:`ERROR_NO_LOCK`.
    :returns: The description string.
    """
    try:
        return _CODE_ERROR[enum]
    except KeyError:
        return enum

def get_error_string_from_enum(enum):
    """Get a short description of an error.

    :param enum: The transaction error enum, e.g. :data:`ERROR_NO_LOCK`.
    :returns: The description string.
    """
    try:
        return _STRINGS_ERROR[enum]
    except KeyError:
        return None


def get_source_name_from_enum(enum):
    try:
        return SOURCE_NAME[enum]
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
        return "cn.kylinos.KylinSystemUpdater.action" #默认配置

# vim:ts=4:sw=4:et
