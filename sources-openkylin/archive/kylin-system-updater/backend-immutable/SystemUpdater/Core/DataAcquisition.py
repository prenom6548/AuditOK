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

# DataAcquisition.py
# supervisory control and data acquisition

#!/usr/bin/python3
import os
import json
import dbus
import socket
import base64
import hashlib
import logging
import datetime

from datetime import datetime
from binascii import a2b_hex
from SystemUpdater.packageManager.pmEnums import *
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *
from json.decoder import JSONDecodeError
from dbus.exceptions import DBusException
from SystemUpdater.Core.UpdaterConfigParser import UpgradeConfig

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes

LOCALTIDDIR  = "/var/lib/kylin-system-updater/"
LOCALTIDFILE = "tidfile.conf"
MSGSNDDIR    = "/var/lib/kylin-system-updater/sendinfos/"
SERVERID = "/var/lib/kylin-software-properties/config/serverID.conf"
DMI_UUID_FILE = "/sys/devices/virtual/dmi/id/product_uuid"

class UpdateMsgCollector():
    ACTION_DEFUALT_STATUS       =-1
    ACTION_UPDATE               = 0
    ACTION_INSTALL              = 1
    ACTION_INSTALL_DEB          = 2
    ACTION_CHECK_RESOLVER       = 3
    ACTION_DOWNLOADONLY         = 4
    ACTION_FIX_BROKEN           = 5
    ACTION_REMOVE_PACKAGES      = 6
    ACTION_FIX_INCOMPLETE       = 7
    ACTION_CLEAN                = 8
    ACTION_INSTALL_SHUTDOWN     = 9

    messageType_map = {
        ROLE_UPDATE_CACHE   :   "UpdateDetect",
        ROLE_DOWNLOAD       :   "Downloaded",
        ROLE_DEPLOY         :   "UpdateInfos"
    }

    ERR_PARA_FROMAT     = 1
    ERR_NO_LOACLTID     = 2
    ERR_ABNORMAL_SHA    = 3
    ERR_UPLOADMSG_SHA   = 4
    ERR_UPLOADMSG_CTS   = 5

    def __init__(self, manager = None):
        self.UploadMessage  = {}
        self.PackageInfo    = {}
        self.updateManager  = manager
        self.convertor      = FormatConvert(self)


    def upgrade_process_msg(self, action, dict_msg = {}):
        try:
            if 'error_string' in dict_msg.keys() and 'error_desc' in dict_msg.keys():
                if dict_msg['error_string'] == "下载失败" and dict_msg['error_desc'] == "取消升级":
                    dict_msg.update({"error_string":str(dict_msg['error_string']+","+dict_msg['error_desc'])})
                    dict_msg.update({"error_desc":str(dict_msg['error_string']+","+dict_msg['error_desc'])})

            self._update_msg(self.messageType_map.get(action, ""), dict_msg.copy())

        except Exception as e:
            logging.error(e)

    def upgrade_process_msg_trans(self, trans):
        UpdateInfos     = {}
        UpdaterVersion  = ''
        appname         = ''

        try:
            UpdaterVersion = self._get_installed_from_dpkg("kylin-system-updater")
            if not UpdaterVersion:
                UpdaterVersion = self._get_installed_from_dpkg("kylin-system-updater")

            if trans.update_metadata:
                appname = str(trans.update_metadata.get("update-type", '')).strip('"\'')
                if not appname:
                    logging.warning("upgrade_process_msg_trans: trans.update_metadata['update-type'] is empty.")
                    return
            else:
                logging.warning("upgrade_process_msg_trans: trans.update_metadata is empty.")
                return

            UpdateInfos.update({"appname"           : appname})
            UpdateInfos.update({"old_version"       : self.updateManager.sqlite3_server.deb_metadata['old_update_version']})
            UpdateInfos.update({"new_version"       : self.updateManager.sqlite3_server.deb_metadata['new_update_version']})
            UpdateInfos.update({"from_revision"     : trans.from_revision})
            UpdateInfos.update({"new_revision"      : trans.new_revision})
            UpdateInfos.update({"source"            : "kylin-system-updater"})
            UpdateInfos.update({"updater_version"   : UpdaterVersion})
            UpdateInfos.update({"status"            : "success" if trans.success else "failed"})
            UpdateInfos.update({"errorCode"         : str(trans.error) if trans.error else ''})
            UpdateInfos.update({"error_string"      : get_error_description_from_enum(trans.error)})
            UpdateInfos.update({"error_desc"        : ""})
            UpdateInfos.update({"old_update_version"    : self.updateManager.sqlite3_server.deb_metadata['old_update_version']})
            UpdateInfos.update({"new_update_version"    : self.updateManager.sqlite3_server.deb_metadata['new_update_version']})

            self._update_msg(self.messageType_map.get(trans.role, ""), UpdateInfos.copy())
        except Exception as e:
            logging.error("upgrade_process_msg_trans error: %s", e)

    def _update_msg(self, messageType, dict_message):
        if messageType == "":
            messageType = "SystemUpdate"

        if messageType == "UpdateInfos":    # 获取系统注册码
            dict_message.update({"RegisterNumber" : self._get_register_number() })

        if type(dict_message) != type(dict) and "appname" not in dict_message.keys():
            raise AttributeError("'%s' object has no attribute '%s'" % ("dict message", "appname"))

        # 生成UploadMessage与PackageInfo
        self._generate_package_info(messageType, "kylin-system-updater")
        self._generate_upload_message(dict_message)

        # 上报接口
        self._update_msg_daq_server()

        self._update_msg_upload_upgrade_status(messageType)

        self._update_msg_log_transmit(messageType)


    def _generate_package_info(self, messageType, packageName):
        PackageInfo = {}

        try:
            PackageInfo.update({"messageType"   : messageType})
            PackageInfo.update({"packageName"   : packageName})
            PackageInfo.update({"tid"           : self._get_local_tid(packageName + '_' + messageType)})

            json_PackageInfo = self.convertor.dictConvertJson(PackageInfo)

            logging.info('Generate PackageInfo: %s.', json_PackageInfo)
        except Exception as e:
            logging.error("_generate_package_info err: %s", e)

        self.PackageInfo = PackageInfo.copy()
        PackageInfo.clear()

    def _generate_upload_message(self, dict_message):
        UploadMessage = {}

        try:
            if "packageName" in dict_message.keys():
                dict_message.pop("packageName")

            for key in dict_message.keys():
                UploadMessage.update({key   : dict_message[key]})

            UploadMessage.update({"createTimeStamp" : create_time_stamp()})
            UploadMessage.update({"UUID"            : self._get_product_uuid()})
            # UploadMessage.update({"serialNumber"    : self._get_serial_number()})
            UploadMessage.update({"serialNumber"    : ''})

            serverid_configs = UpgradeConfig(datadir = "/var/lib/kylin-system-updater/transID/", name = "serverID.conf")
            UploadMessage.update({"transactionID"   : serverid_configs.getWithDefault("ID", "pushID", "null")})
            logging.debug('Get current transactionID: %s.', UploadMessage['transactionID'])

            updateid_configs = UpgradeConfig(datadir = "/var/lib/kylin-system-updater/transID/", name = "updateID.conf")
            UploadMessage.update({"updateID"   : updateid_configs.getWithDefault("update", "uuid", "")})
            logging.debug('Get current updateID: %s.', UploadMessage['updateID'])

            json_UploadMessage = self.convertor.dictConvertJson(UploadMessage)

            logging.info('Generate UploadMessage: %s.', json_UploadMessage)
        except Exception as e:
            logging.error("_generate_upload_message err: %s", e)

        self.UploadMessage = UploadMessage.copy()
        UploadMessage.clear()

    def _update_msg_daq_server(self):
        try:
            json_UploadMessage = self.convertor.dictConvertJson(self.UploadMessage)
            json_PackageInfo   = self.convertor.dictConvertJson(self.PackageInfo)

            shaValue  = self.convertor.Sha256Value(json_UploadMessage)
            encodeMsg = self.convertor.EncodeRSAtoBase64(shaValue)

            self.MsgSendToServer(json_UploadMessage, json_PackageInfo, encodeMsg)
        except Exception as e:
            logging.error(e)

    def _update_msg_upload_upgrade_status(self, messageType):
        try:
            status = False
            if "status" in self.UploadMessage.keys():
                status = True if str(self.UploadMessage["status"]).lower() == "success" \
                              or str(self.UploadMessage["status"]).lower() == "true" \
                              else False

            if messageType == "Downloaded":
                statusCode = 3 if status else -3
            elif messageType == "UpdateInfos" or messageType == "selfUpgrade":
                statusCode = 4 if status else -4
            elif messageType == "Background-Upgrade" or messageType == "Background":
                statusCode = 5 if status else -5
            else:
                return

            errorString = ""
            if "error_string" in self.UploadMessage.keys():
                errorString = errorString + " " + self.UploadMessage["error_string"]
            if "error_desc" in self.UploadMessage.keys():
                errorString = errorString + " " + self.UploadMessage["error_desc"]

            uuid = ""
            if "updateID" in self.UploadMessage.keys():
                uuid = self.UploadMessage["updateID"]

            if messageType == "selfUpgrade" and "appname" in self.UploadMessage.keys():
                errorString = self.UploadMessage["appname"]
                uuid = 'jm0qhvdz3g'

            self.MsgSendToUploadUpgradeStatus(uuid, statusCode, errorString)
        except Exception as e:
            logging.error(e)

    def _update_msg_log_transmit(self, messageType):
        try:
            if messageType != "UpdateInfos" and messageType != "Background" and \
               messageType != "P2PInfos" and messageType != "Background-Upgrade":
                return

            if "appname" in self.UploadMessage.keys():
                self.UploadMessage["packageName"] = self.UploadMessage["appname"]
                self.UploadMessage.pop("appname")

            if "messageType" in self.PackageInfo.keys():
                if self.PackageInfo["messageType"] != "P2PInfos":
                    if self.PackageInfo["messageType"] == "Background":
                        self.UploadMessage.update({"upgrade_mode": str("强制更新")})
                    else:
                        self.UploadMessage.update({"upgrade_mode": str("非强制更新")})

                TYPE_INSTALL = "1"
                TYPE_UPGRADE = "2"
                TYPE_REMOVE  = "3"

                if "packageName" in self.UploadMessage.keys():
                    if self.UploadMessage["packageName"] == "kylin-update-desktop-system":
                        self.UploadMessage.update({"type": TYPE_UPGRADE})
                    else:
                        if "old_version" in self.UploadMessage.keys():
                            if self.UploadMessage["old_version"] == "":
                                self.UploadMessage.update({"type": TYPE_INSTALL})
                            else:
                                self.UploadMessage.update({"type": TYPE_UPGRADE})

            json_UploadMessage = self.convertor.dictConvertJson(self.UploadMessage)
            json_PackageInfo   = self.convertor.dictConvertJson(self.PackageInfo)

            shaValue = self.convertor.Sha256Value(json_UploadMessage) 
            encodeMsg = self.convertor.EncodeRSAtoBase64(shaValue)

            self.MsgSendToLogTransmit(json_UploadMessage, json_PackageInfo, encodeMsg)
        except Exception as e:
            logging.error(e)


    def MsgSendToServer(self, UploadMessage, PackageInfo, encodeMsg):
        daqbus = dbus.SystemBus()
        try:
            daqobj = daqbus.get_object('com.kylin.daq', '/com/kylin/daq')
            daqinterface = dbus.Interface(daqobj, dbus_interface='com.kylin.daq.interface')
        except DBusException as e:
            logging.error("kylin-daq service error: "+str(e))
            return 
        try:
            retval,retid = daqinterface.UploadMessage(PackageInfo, UploadMessage, encodeMsg)
        except AttributeError:
            logging.error("Call UploadMessage: Attribute Error.")
        self.Send_finally(retval, retid, PackageInfo)

    def MsgSendToUploadUpgradeStatus(self, uuid, statusCode, description):
        try:
            daqbus = dbus.SystemBus()
            daqobj = daqbus.get_object('com.kylin.software.properties', '/com/kylin/software/properties')
            daqinterface = dbus.Interface(daqobj, dbus_interface='com.kylin.software.properties.interface')
        except DBusException as e:
            logging.warning("kylin-daq service error: "+str(e))
            return
        try:
            logging.info("uploadUpgradeStatus message: uuid:%s,statusCode:%s,description:%s.",uuid,statusCode,description)
            retval = daqinterface.uploadUpgradeStatus(uuid, statusCode, description)
            logging.info("uploadUpgradeStatus message retval: %s.", retval)
        except Exception as err:
            logging.error("Call uploadUpgradeStatus: %s.",err)

    def MsgSendToLogTransmit(self, UploadMessage, PackageInfo, encodeMsg):
        daqbus = dbus.SystemBus()
        hedronbus = dbus.SystemBus() 
        messgae = {}
        info = {}
        hostname = ""
        try:
            daqobj = daqbus.get_object('org.log.sys_transmit', '/org/log/sys_transmit')
            daqinterface = dbus.Interface(daqobj, dbus_interface='org.log.transmit')
        except DBusException as e:
            logging.warning("sys_transmit service error: "+str(e)) #主线默认不通过安全管控上传日志
            return 
        
        dict_UploadMessage = self.convertor.JsonConvertDict(UploadMessage)
        dict_PackageInfo = self.convertor.JsonConvertDict(PackageInfo)

        messgae.update({"packageName": str(dictgetattr(dict_PackageInfo, "packageName", 'None'))})
        messgae.update({"messageType": str(dictgetattr(dict_PackageInfo, "messageType", 'None'))})
        for k in dict_UploadMessage.keys():
            messgae.update({k: str(dictgetattr(dict_UploadMessage, k, 'unKnown'))})
        if "status" in messgae.keys():
            if messgae["status"] == "success":
                messgae.update({"status": str("成功")})
            else:
                messgae.update({"status": str("失败")})
        json_messgae = self.convertor.dictConvertJson(messgae)

        info.update({"time":str(dictgetattr(dict_UploadMessage, 'createTimeStamp', ' '))})
        
        try:
            TESTIPADDR = os.getenv('TESTIPADDR', '8.8'+'.'+'8.8')
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect((TESTIPADDR, 80))
            host_ip = s.getsockname()[0]
        except:
            host_ip = '0.0.0.0'
        try:
            hedronobj = hedronbus.get_object('com.kylin.kydevmonit.hedronclient', '/kydevmonit/hedronclient')
            hedroninterface = dbus.Interface(hedronobj, dbus_interface='com.kylin.kydevmonit.hedronclient')
            hostname = hedroninterface.get_current_user()
        except Exception as err:
            logging.warning(err)
            hostname = ""
        info.update({"hostname":str(hostname)})
        info.update({"ip":str(host_ip)})
        info.update({"name":str("kylin-system-updater")})
        info.update({"lv":str("debug")})
        info.update({"message":json_messgae})
        json_info = self.convertor.dictConvertJson(info)
        logging.info("Send to sys_transmit: < %s >.",json_info)
        try:
            retval = daqinterface.log_transmit(json_info)
            logging.info("Send retval: %s.", retval)
        except Exception as err:
            logging.error(err)

    def Send_finally(self, retval, retid, json_PackageInfo):
        # 根据发送结果进行处理
        result = ''
        PackageInfo = self.convertor.JsonConvertDict(json_PackageInfo)
        if retval != 0: 
            if retval == self.ERR_PARA_FROMAT:
                result = "Parameter format error"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            elif retval == self.ERR_NO_LOACLTID:
                result = "The tid value in packageInfo is abnormal, but the message is saved successfully"
                logging.info("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
                # 将返回的tid保存到本地
                key = PackageInfo['packageName']+'_'+PackageInfo['messageType']
                self._save_tid(key, retid)
            elif retval == self.ERR_ABNORMAL_SHA:
                result = "Abnormal UploadedMessage Sha256"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            elif retval == self.ERR_UPLOADMSG_SHA:
                result = "Description The UploadedMessageSha256 was decrypted incorrectly"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            elif retval == self.ERR_UPLOADMSG_CTS:
                result = "The createTimeStamp field of UploadedMessage is abnormal"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            elif retval == self.ERR_UPLOADMSG_CTS:
                result = "Invalid key included in \"uploadedMessage\" or \"packageInfo\": <@timestamp>,<_id>,<_index>,<_type>,<createTime>,<highlight>,<sn>,<sort>, check upload field"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            else:
                logging.error("Sent Status: false - packageName: %s : retval: %s.", PackageInfo['packageName'], retval)
        elif retval == 0:
            result = "Send to server success"
            logging.info("Sent Status: True - packageName: %s : result: %s.", PackageInfo['packageName'], result)

    def Msg_Clean(self):
        self.UploadMessage  = {}
        self.PackageInfo    = {}


    def _get_local_tid(self, key):
        localtid = ""

        try:
            tid = self.updateManager.sqlite3_server.select_from_tid("tid", key)
            if tid == "None" or tid == None:
                localtid = ""
            else:
                localtid = tid
        except Exception as e:
            logging.error("_get_local_tid err: %s", e)
        
        return localtid

    def _save_tid(self, key, localtid):
        try:
            if not localtid:
                return

            _localtid = str(localtid)
            self.updateManager.sqlite3_server.insert_into_tid(key, _localtid)
        except Exception as e:
            logging.error("_save_tid err: %s", e)

    def _get_product_uuid(self):
        uuid = ""

        try:            
            if os.path.isfile(DMI_UUID_FILE):
                with open(DMI_UUID_FILE, 'r') as f:
                    data = f.read()
                    uuid = data.strip()

            if not uuid:
                args = ["/usr/sbin/dmidecode","-s","system-uuid"]

                from subprocess import Popen, PIPE
                p = Popen(args, stdout=PIPE, universal_newlines=True)
                res = p.wait()
                if res != 0:
                    uuid = ""
                else:
                    uuid = p.stdout.readline().strip()
                p.stdout.close()

            if not uuid:
                import uuid
                uuid = str(uuid.uuid1())
        except Exception as e:
            logging.error("_get_product_uuid err: %s", e)

        return uuid

    def _get_serial_number(self):
        serial_number = ''
        self.restore_serial_number = ''
        try:
            snobj = dbus.SystemBus().get_object('org.freedesktop.activation', '/org/freedesktop/activation')
            sninterface = dbus.Interface(snobj, dbus_interface='org.freedesktop.activation.interface')

            retval, retid = sninterface.serial_number()
            if not retval:
                retval, retid = sninterface.serial_number()

            if (retid == 0):
                serial_number = str(retval)
            else:
                if os.path.exists("/etc/.kyinfo"):
                    kyinfo_configs = UpgradeConfig(datadir = "/etc/", name = ".kyinfo")
                    serial_number = kyinfo_configs.getWithDefault("servicekey", "key", "0")
            
            if not serial_number and self.restore_serial_number:
                serial_number = self.restore_serial_number

            self.restore_serial_number = serial_number
        except Exception as e:
            logging.error("_get_serial_number err: %s", e)

        return serial_number
    
    def _get_register_number(self):
        Code = ''
        try:
            bus = dbus.SystemBus()
            obj = bus.get_object('org.freedesktop.activation', '/org/freedesktop/activation')
            interface = dbus.Interface(obj, dbus_interface='org.freedesktop.activation.interface')
            retval,ret = interface.register_number()
            if ret == 0 and len(retval) != 0:
                Code = str(retval)
            elif len(retval) == 0:  # sh 查询
                sh_retval = os.popen("kylin_gen_register").read().strip()
                if len(sh_retval) != 0:
                    Code = str(sh_retval)
        except Exception as e:
            logging.error(str(e))
            return ""
        return Code

    def extract_info(self,line):
        parts = line.split()
        if not parts:
            return None, None
        else:
            return parts[1], parts[2]

    def _parse_dpkg_output(self, packageName, output):
        data_version = ''
        lines = output.splitlines()

        for line in lines:
            if packageName in line:
                name, version = self.extract_info(line.strip())
                if name and version:
                    if ":" in name:
                        name = name.split(":")[0]
                    if ":" in version:
                        version = version.split(":")[-1]
                    data_version = version

        return data_version

    def _get_installed_from_dpkg(self, packageName = ''):
        import subprocess

        local_packages = None
        process = subprocess.Popen("dpkg -l "+packageName, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, error = process.communicate()
        if not error:
            local_packages = self._parse_dpkg_output(packageName, output.decode('utf-8'))
        else:
            logging.warning(str(error))

        return local_packages


class FormatConvert():
    def __init__(self, DataCollector):
        #秘钥
        self.publickey = UniqueKey()
        self.collector = DataCollector

    def dictConvertJson(self, dict_msg):
        #字典转换为json格式字符串
        json_file = ''
        try:
            json_file = json.dumps(dict_msg,ensure_ascii=False)
        except JSONDecodeError as e:
            logging.error(str(e))
        return json_file
    
    def JsonConvertDict(self, json_file):
        # json格式字符串转换为字典
        dict_file = {}
        try:
            dict_file = json.loads(json_file)
        except JSONDecodeError as e:
            logging.error(str(e))
        return dict_file
    
    def Sha256Value(self, json_file):
        # 计算sha256值
        hsobj = hashlib.sha256()
        try:
            hsobj.update(json_file.encode("utf-8"))
        except ValueError as e:
            logging.error("SHA256 value error: %s.",str(e))
        except Exception as e:
            logging.error(str(e))
        
        return hsobj.hexdigest()

    def EncodeRSAtoBase64(self, value):
        # 将value进行RSA加密并base64转码
        enMsg = ""
        try:
            # 加载公钥
            public_key = serialization.load_pem_public_key(
                base64.b64decode(self.publickey.keyvalue.encode('utf-8')),
                backend=default_backend()
            )

            # 加密数据
            encodemsg = public_key.encrypt(
                a2b_hex(value),
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA1()),
                    algorithm=hashes.SHA1(),
                    label=None
                )
            )

            # 加密数据Base64转码
            enMsg = base64.b64encode(encodemsg)

        except Exception as e:
            logging.error("EncodeRSAtoBase64 error: %s.", e)
        return enMsg


class UniqueKey():
    keyvalue = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FR\
OEFNSUlCQ2dLQ0FRRUFzdW1NTFJEdlFNb0tEQkRJODRqSgpqc1A0Mk55V0pWVEZob2Jra3ZiT05j\
dExYTXVzRmo2TzJUblZYU3Z6VlVLSjRqZkpwT2l2WEphOVB5Z2wzYTRnClBzSU40enNCMEdOY0tr\
R3VsS2RrV2x6S3lWQ2xlTzhiQnN6SjkwbTc3cWF6YWg3a1A0TUl0WTVFczBpSkpiR0oKN1MxcERj\
MlJkNnVFQWJLaXJyRTFlNzlFTEd4am5VN2V5NWkyRDE2WWJoZEQwZ2lNa2RHR3piQXBKTWZWRVJR\
TQo1NXorMFVqdS8zSFJhNFY3b3p2TGRPRE5HUURaeWNJU0l3VHBLbFR3RjBxazdCNjVhTUlJenQ1\
dnhOK1lxYU1GClppZFRLNzcxNjdqNEExZ3F3MG45bjlybWVXUGRWZ3dudnRtVXp4Q1krNk05SXpK\
TDI3eWpRUTV1WGQ3RVdMT3IKbndJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="


def create_time_stamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]


def dictgetattr(_dict, _key, default_value):
    if type(_dict) != type(dict()):
        return 
    if _key in _dict.keys():
        return _dict[_key]
    else :
        return default_value


if __name__ == "__main__":
    pass
