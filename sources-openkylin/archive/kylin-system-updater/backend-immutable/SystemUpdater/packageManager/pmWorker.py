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

import logging
from gettext import gettext as _

from SystemUpdater.packageManager.pmUtils import *
from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig
from SystemUpdater.packageManager.UpdaterSDKInterface import UpdaterSDKPkgInstall

class PmWorker():
    def __init__(self, parent = None):
        self.parent             = parent
        self.dbus_send          = parent.dbus_send
        self.sqlite3_server     = parent.sqlite3_server
        self.updater_package    = UpdaterSDKPkgInstall ( parent )

        if not hasattr(parent, 'configs_uncover') or parent.configs_uncover is None:
            self.configs_uncover    = UpgradeConfig (
                                        defaults_conf = "/etc/kylin-system-updater/system-updater-defaults.conf" )
        else:
            self.configs_uncover    = parent.configs_uncover

    def start_deb_install(self, deb_uri = "", _check_local_dep = False, _auto_satisfy = False, source = '', sender=None):
        sender_name = get_proc_from_dbus_name(sender)
        caller_trans = get_source_name_from_enum(sender_name)

        try:
            def _run_install_handler():
                _success = self.updater_package.package_interface_install(deb_uri)
                if self.updater_package.installFinished:
                    self.updater_package.installFinished = False
                else:
                    self.dbus_send.InstalldebFinished(_success, '', '')
                    self.dbus_send.InstalldebFinishedWithErrCode(_success, '', '', '')

            def on_authorization_success(result):
                logging.info("Authorization success ...")

                self.sqlite3_server.deb_policy_keep = True
                def _check_deb_policy():
                    if self.sqlite3_server.deb_policy_timestamp % 10 == 0:
                        logging.info("Checking for deb policy timeout(%d)...",self.sqlite3_server.deb_policy_timestamp)
                    if (self.sqlite3_server.deb_policy_timestamp <= 0):
                        logging.warning("Deb policy timeout")
                        self.sqlite3_server.deb_policy_keep = False
                        return False
                    else:
                        self.sqlite3_server.deb_policy_timestamp = self.sqlite3_server.deb_policy_timestamp - 1
                    return True

                from gi.repository import GLib
                self.sqlite3_server.deb_policy_timestamp = 60 * 5
                GLib.timeout_add_seconds(1,_check_deb_policy)

                _run_install_handler()

            def on_authorization_failure(error):
                try:
                    logging.info("Authorization failed ...")

                    self.sqlite3_server.deb_policy_keep = False
                    self.sqlite3_server.deb_policy_timestamp = 0

                    error.raise_exception()
                except Exception as error:
                    logging.error(error)
                    self.dbus_send.InstalldebFinished(False,_("Cancel authentication."),'')
                    self.dbus_send.InstalldebFinishedWithErrCode(False,_("Cancel authentication."),'','')

            def _policykit_authority_handler(install_handler = None, authority_failed_handler = None):
                if not self.sqlite3_server.deb_policy_keep:
                    deferred = PolicyKit_Authority(caller_trans+_(" requires authentication to install software packages."),
                                                sender = sender,
                                                authentication = self.configs_uncover.getWithDefault(
                                                                                        "InstallAndPurge",
                                                                                        "install_authority", False),
                                                source=source)

                    deferred.add_callback(on_authorization_success)
                    deferred.add_errback(on_authorization_failure)
                else:
                    self.sqlite3_server.deb_policy_timestamp = 60 * 5
                    logging.info("Deb policy keep, ignore...")

                    _run_install_handler()

            if not deb_verify(deb_uri):
                _policykit_authority_handler()
            else:
                _run_install_handler()

        except Exception as e:
            logging.info("start_deb_install error: %s", e)
            self.dbus_send.InstalldebFinished(False, str(e), '')
            self.dbus_send.InstalldebFinishedWithErrCode(False, str(e), '', '')

    def start_purge_pkgs(self,pkgs_list,sender=None):
        try:
            sender_name = get_proc_from_dbus_name(sender)

            def _run_purge_handler():
                for pl in pkgs_list:
                    _success = self.updater_package.package_interface_remove(pl)
                    if not _success:
                        break

                if self.updater_package.purgeFinished:
                    self.updater_package.purgeFinished = False
                else:
                    self.dbus_send.PurgePackagesFinished (_success, '', '')
                    self.dbus_send.PurgePackagesFinishedWithPkgname (_success, '', '', ",".join(pkgs_list))
                    self.dbus_send.PurgePackagesFinishedWithErrCode (_success, '', '', ",".join(pkgs_list), '')

            def on_authorization_success(result):
                logging.info("Authorization success ...")

                self.sqlite3_server.purge_policy_keep = True
                def _check_purge_policy():
                    if self.sqlite3_server.purge_policy_timestamp % 10 == 0:
                        logging.info("Checking for deb policy timeout(%d)...",self.sqlite3_server.purge_policy_timestamp)
                    if (self.sqlite3_server.purge_policy_timestamp <= 0):
                        logging.warning("Deb policy timeout")
                        self.sqlite3_server.purge_policy_keep = False
                        return False
                    else:
                        self.sqlite3_server.purge_policy_timestamp = self.sqlite3_server.purge_policy_timestamp - 1
                    return True

                from gi.repository import GLib
                self.sqlite3_server.purge_policy_timestamp = 60 * 5
                GLib.timeout_add_seconds(1,_check_purge_policy)

                _run_purge_handler()

            def on_authorization_failure(error):
                """Convert the DeferredException to a normal exception."""
                try:
                    logging.info("Authorization failed ...")

                    self.sqlite3_server.purge_policy_keep = False
                    self.sqlite3_server.purge_policy_timestamp = 0

                    error.raise_exception()
                except Exception as error:
                    logging.error(error)
                    self.dbus_send.PurgePackagesFinished(False,_("Cancel authentication."),'')
                    self.dbus_send.PurgePackagesFinishedWithPkgname(False,_("Cancel authentication."),'',",".join(pkgs_list))
                    self.dbus_send.PurgePackagesFinishedWithErrCode(False,_("Cancel authentication."),'',",".join(pkgs_list),get_error_num_from_enum(ERROR_PURGE_PKG_VERIFY))

            def _policykit_authority_handler(install_handler = None, authority_failed_handler = None):
                if not self.sqlite3_server.purge_policy_keep:
                    deferred = PolicyKit_Authority(
                                    get_source_name_from_enum(sender_name)+_(" requires authentication to uninstall software packages."),
                                    sender = sender,
                                    authentication = self.configs_uncover.getWithDefault("InstallAndPurge","purge_authority",False), 
                                    source=sender_name)

                    deferred.add_callback(on_authorization_success)
                    deferred.add_errback(on_authorization_failure)
                else:
                    self.sqlite3_server.purge_policy_timestamp = 60 * 5
                    logging.info("Purge policy keep, ignore...")

                    _run_purge_handler()

            _policykit_authority_handler()

        except Exception as e:
            logging.error("start_purge_pkgs error: %s", e)

    def start_back_upgrade(self, pkgs_list, sender=None):
        _success        = False

        try:
            batchInstPkgs = [str(item) for item in pkgs_list]

            _success = self.updater_package.package_interface_batch_install(batchInstPkgs)
            if self.updater_package.batchInstFinished:
                self.updater_package.batchInstFinished = False
            else:
                self.dbus_send.UpdateInstallFinished(_success, batchInstPkgs, '', '')
                self.updater_package.package_interface_install_clear()

        except Exception as e:
            logging.error("start_back_upgrade err: %s", e)

    def check_package_status(self, _name, _version = '', sender = None):
        install_status = -100

        try:
            if not _version:
                _version = 'NULL'

            install_status = self.updater_package.package_interface_check_status(_name, _version)

        except Exception as e:
            logging.error("check_package_status err: %s", e)

        return install_status
