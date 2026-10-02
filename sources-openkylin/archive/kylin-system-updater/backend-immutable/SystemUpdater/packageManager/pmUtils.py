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
import dbus
import psutil
import logging
import subprocess
from ctypes import *
from defer import Deferred
from gettext import gettext as _
from apt.debfile import DebPackage
from SystemUpdater.Core.errors import *
from SystemUpdater.packageManager.pmEnums import *

def deb_format_check(deb_uri):
    format_check = False

    try:
        if not os.path.isfile(deb_uri):
            return format_check

        DebPackage(deb_uri)
        format_check = True
        logging.info("deb_format_check '%s' legal", deb_uri)
    except Exception as e:
        logging.error(e)
        
    return format_check

def deb_verify(deb_path):
    logging.info("Verify pkg:%s.",deb_path)
    _deb_path = str(deb_path)
    _verify_status = False
    try:
        if not deb_format_check(_deb_path) or not kysec_authority():
            _verify_status = True
            return _verify_status

        # # 加载验证签名库 , 验签接口暂时无法调用
        if not os.path.isfile("/usr/bin/kylinsigntool"):
            logging.error("SOF_InitializeEx error!")
            return _verify_status
        args = ["/usr/bin/kylinsigntool", "-v", _deb_path]
        ret = subprocess.run(args, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        if "Signature Verified failed" in str(ret.stdout).strip() or "签名验证失败" in str(ret.stdout).strip() \
                or "Deb signature does not exist" in str(ret.stdout).strip() or "签名不存在" in str(ret.stdout).strip() \
                or "证书验证失败" in str(ret.stdout).strip():
            logging.info("Signature Verified failed!")
        elif "Signature Verified Ok" in str(ret.stdout).strip() or "签名验证成功" in str(ret.stdout).strip() \
                or "Certificate verification is successful" in str(ret.stdout).strip() or "证书验证成功" in str(ret.stdout).strip():
            logging.info("Signature Verified Ok!")
            _verify_status = True
        else:
            logging.error("Signature Verified failed:%s.",ret)
    except Exception as e:
        logging.error(e)
    return _verify_status


def PolicyKit_Authority(details = '', sender = None, authentication = False, source=''):
    deferred = Deferred()
    try:        
        #用户鉴权
        logging.info("Authentication via PolicyKit .")
        details = {'polkit.message':details}
        cancel_id = ''

        if False:
            source=source+'-self'

        logging.info('authentication status: %r.',authentication)
        if True == authentication:
            subject = ('system-bus-name', {'name': sender})
            action = get_policykit_authority_action_enum(source)

            def policykit_done(xxx_todo_changeme):
                (authorized, challenged, auth_details) = xxx_todo_changeme
                if authorized:
                    deferred.callback(auth_details)
                elif challenged:
                    deferred.errback(AuthorizationFailed(subject, action))
                else:
                    deferred.errback(NotAuthorizedError(subject, action))

            logging.info("PolicyKit source: %s, action: %s.",source,action)
            kit = dbus.SystemBus().get_object('org.freedesktop.PolicyKit1', '/org/freedesktop/PolicyKit1/Authority')
            kit = dbus.Interface(kit, 'org.freedesktop.PolicyKit1.Authority')
            kit.CheckAuthorization(subject, action, details, 
                                dbus.UInt32(1), action, 
                                timeout=60*60*24*7,
                                reply_handler=policykit_done,
                                error_handler=deferred.errback)
        else:
            deferred.callback(_("Authentication success."))
    except Exception as e:
        logging.error(e)
        deferred.errback(AuthorizationFailed(subject, action))
    return deferred

def kysec_authority():
    _allow_kylinsign        = False
    _verify_kylinsign       = False
    _kysec_authority_status = True

    try:
        #获取未知来源应用安装策略Unknown sources apply installation policies
        inst_policies_path = "/etc/dpkg/dpkg.cfg"
        if os.path.isfile(inst_policies_path):
            with open(inst_policies_path, "r") as f:
                lines = f.readlines()
            for line in lines:
                if "allow-kylinsign" in line and "#allow-kylinsign" not in line:
                    _allow_kylinsign = True
                if "verify-kylinsign" in line and "#verify-kylinsign" not in line:
                    _verify_kylinsign = True

            if _allow_kylinsign == True and _verify_kylinsign == False: #策略： 阻止
                logging.info("unknown sources apply installation policies: deter")
                _kysec_authority_status = False

            elif _allow_kylinsign == True and _verify_kylinsign == True: #策略： 警告
                logging.info("unknown sources apply installation policies: warning")

            elif _allow_kylinsign == False and _verify_kylinsign == False: #策略： 关闭
                logging.info("unknown sources apply installation policies: close")
                _kysec_authority_status = False

    except Exception as e:
        logging.error(e)

    return _kysec_authority_status


class NotAuthorizedError(dbus.DBusException):

    _dbus_error_name = "org.freedesktop.PolicyKit.Error.NotAuthorized"

    def __init__(self, subject, action_id):
        dbus.DBusException.__init__(self, "%s: %s" % (subject, action_id))
        self.action_id = action_id
        self.subject = subject


class AuthorizationFailed(NotAuthorizedError):
    _dbus_error_name = "org.freedesktop.PolicyKit.Error.Failed"


def ApplicationSecInstall(debname = ''):

    SEC_MODE_NORMAL = 0
    SEC_MODE_WHITELIST = 1
    SEC_MODE_BLACKLIST = 2

    try:
        logging.debug("Security Application Install Check .")

        sec = dbus.SystemBus().get_object('com.kylin.kysdk.applicationsec', '/com/kylin/kysdk/applicationsec')
        secinterface = dbus.Interface(sec, dbus_interface='com.kylin.kysdk.applicationsec.antiinstall')
        secMode = secinterface.GetMode()

        if SEC_MODE_NORMAL == secMode:
            logging.info("Application installation control policy not enabled .")
            return True,_("Application installation control policy not enabled .")

        elif SEC_MODE_WHITELIST == secMode:
            logging.info("Application installation in whitelist mode .")

            if secinterface.PackageIsInWhilteList(debname) :
                logging.info("Application in whitelist .")
                return True,""
            else:
                logging.info("Application not in whitelist .")
                return False,_("Installation failed! Application is not in the software whitelist list!")

        elif SEC_MODE_BLACKLIST == secMode:
            logging.info("Application installation in blacklist mode .")
            
            if secinterface.PackageIsInBlackList(debname) :
                logging.info("Application in blacklist .")
                return False,_("Installation failed! Application is in the software blacklist list!") 
            else:
                logging.info("Application not in blacklist .")
                return True,""

        else:
            logging.info("Application installation in unknown mode .")
            return True,_("Application installation in unknown mode .")

    except dbus.exceptions.DBusException as e:
        dbus_error = str(e._dbus_error_name)
        compare_error = str("org.freedesktop.DBus.Error.ServiceUnknown")
        if dbus_error == compare_error:
            logging.info("Application installation control policy not enabled .")
            return True,"Application installation control policy not enabled ."
        raise

    except Exception as e:
        logging.error(e)
        return False,str(e)

def get_proc_from_dbus_name(dbus_name, bus=None):
    """Return a deferred that gets the id of process owning the given
    system D-Bus name.
    """

    if not bus:
        bus = dbus.SystemBus()
    bus_obj = bus.get_object("org.freedesktop.DBus",
                            "/org/freedesktop/DBus/Bus")
    pid = bus_obj.GetConnectionUnixProcessID(dbus_name,
                                    dbus_interface="org.freedesktop.DBus")
    proc = psutil.Process(int(pid))

    return proc.name()

if __name__ == "__main__":
    pass
