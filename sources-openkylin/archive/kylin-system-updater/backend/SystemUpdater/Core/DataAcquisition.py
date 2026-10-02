# DataAcquisition.py
# supervisory control and data acquisition

#!/usr/bin/python3
import os
import json
import dbus
import uuid
import time
import socket
import base64
import shutil
import hashlib
import logging
import tarfile
import requests
import datetime
import threading

from datetime import datetime
from binascii import a2b_hex
from urllib import parse, request
from SystemUpdater.Core import enums
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

SOURCE_PKGNAME = {
    'Kylin System Updater': 'kylin-system-updater',
    'Kylin Unattended Upgrade': 'unattended-upgrades'
}

class UpdateMsgCollector():
    ACTION_DEFUALT_STATUS = -1
    ACTION_UPDATE = 0
    ACTION_INSTALL = 1
    ACTION_INSTALL_DEB = 2
    ACTION_CHECK_RESOLVER = 3
    ACTION_DOWNLOADONLY = 4
    ACTION_FIX_BROKEN = 5
    ACTION_REMOVE_PACKAGES = 6
    ACTION_FIX_INCOMPLETE = 7
    ACTION_CLEAN = 8
    ACTION_INSTALL_SHUTDOWN = 9

    MODE_DEFAULT_STATUS = -1
    
    #1、ACTION_INSTALL 安装的子类
    #部分升级
    MODE_INSTALL_PARTIAL = 0
    #全部升级
    MODE_INSTALL_ALL = 1
    #系统全盘升级
    MODE_INSTALL_SYSTEM = 2
    #后端内部安装包使用 目前 更新配置包和升级本身使用
    MODE_INSTALL_SINGLE = 3

    #2、更新的子类
    MODE_UPDATE_CACHE = 0
    MODE_UPDATE_ALL = 1

    mode_map = {
        MODE_INSTALL_PARTIAL:"upgrade_system",
        MODE_INSTALL_ALL:"upgrade_all",
        MODE_INSTALL_PARTIAL:"upgrade_partial"
    }
    action_map = {
        ACTION_CHECK_RESOLVER:enums.MONIT_DEPRESOLUT,
        ACTION_INSTALL:enums.MONIT_INSTALL,
        ACTION_INSTALL_DEB:enums.MONIT_INSTALLDEB,
        10:enums.MONIT_FINISH,
        "finish-update":enums.MONIT_FINISH,
        "finish-install":enums.MONIT_FINISH
    }
    messageType_map = {
        # InstallBackend.ACTION_CHECK_RESOLVER:"UpdateDetect",
        ACTION_CHECK_RESOLVER:"DepResolution",
        # InstallBackend.ACTION_CHECK_RESOLVER:"Downloading",
        ACTION_INSTALL:"Installing",
        # InstallBackend.ACTION_CHECK_RESOLVER:"UpgradeFinish",
        ACTION_INSTALL_DEB:"InstallerInfo",
        10:"Background-Upgrade",
        "finish-update":"UpdateInfos",
        "finish-install":"InstallInfos"
    }

    def __init__(self, manager=None):
        self.uuid = ''
        self.status = ''
        self.upgrade_mode = ''
        self.upgrade_action = ''
        self.UploadMessage = {}
        self.PackageInfo = {}
        self.UpdateInfos = {}
        self.background_version = {}
        self.background_upgradable = []
        self.background_list = []
        self.upgrade_list = []
        self.waitSendList = []
        self.cache = None
        self.updateManager = manager
        # 转换 & 加密
        self.convertor = FormatConvert(self)
        # 发送器
        self.sender = MessageSend(self)
        logging.info("Initialize Update MessageSend Collector to success...")

    def GenUploadMessage(self, dict_message, local_uuid = ''):
        UploadMessage = {}
        # 获取将要上传的数据,获取东八区时间
        UploadMessage['createTimeStamp'] = get_east_8_time()
        try:
            if "packageName" in dict_message.keys():
                dict_message.pop("packageName")
            for key in dict_message.keys():
                UploadMessage[key] = dict_message[key]
            if local_uuid != '':
                UploadMessage['UUID'] = str(local_uuid)
            else:
                UploadMessage['UUID'] = str(uuid.uuid1())
            serverid_configs = UpgradeConfig(datadir = "/var/lib/kylin-software-properties/config/", name = "serverID.conf")
            tans_id = serverid_configs.getWithDefault("ID", "pushID", " ")
            UploadMessage['transactionID'] = tans_id

            snobj = dbus.SystemBus().get_object('org.freedesktop.activation', '/org/freedesktop/activation')
            sninterface = dbus.Interface(snobj, dbus_interface='org.freedesktop.activation.interface')
            retval,retid = sninterface.serial_number()
            if(retid==0):
                UploadMessage['serialNumber'] = str(retval)
            else:
                UploadMessage['serialNumber'] = ''
        except Exception as e:
            logging.error(str(e))

        json_UploadMessage = self.convertor.dictConvertJson(UploadMessage)

        logging.debug('Generate UploadMessage: %s.',json_UploadMessage)
        self.UploadMessage = UploadMessage.copy()
        UploadMessage.clear()

    def GenPackageInfo(self, messageType, packageName):
        PackageInfo = {}
        PackageInfo['messageType'] = str(messageType)
        PackageInfo['packageName'] = str(packageName)
        key = str(packageName)+'_'+str(messageType)
        # 获取本地tid
        self.sender.GetLocalTid(key)
        PackageInfo["tid"] = str(self.sender.localtid)
        
        json_PackageInfo = self.convertor.dictConvertJson(PackageInfo)

        logging.debug('Generate PackageInfo: %s.',json_PackageInfo)
        self.PackageInfo = PackageInfo.copy()
        PackageInfo.clear()

    def setUploadMessage(self, KeyValue):
        # FIXME:数据获取顺序问题
        pass

    def UpdateMsg(self, messageType, json_message, uuid = ''):
        # para: messageType(消息类型):  "UpdateInfos"、 "InstallInfos"、 "RemoveInfo"
        # para: dict_message(数据内容): 必须包含 "packageName"、"source"", 采集器会进行检测
        dict_message = self.convertor.JsonConvertDict(json_message)
        if messageType == "":
            messageType = "SystemUpdate"
        if type(dict_message) != type(dict) and "appname" not in dict_message.keys():
            raise AttributeError("'%s' object has no attribute '%s'" % ("dict message", "appname"))

        # 生成UploadMessage与PackageInfo
        try:
            self.GenPackageInfo(messageType, "kylin-system-updater")
            self.GenUploadMessage(dict_message, local_uuid = uuid)
        except Exception as e:
            logging.error(str(e))
        
        # sha256
        json_UploadMessage = self.convertor.dictConvertJson(self.UploadMessage)
        json_PackageInfo   = self.convertor.dictConvertJson(self.PackageInfo)
        shaValue = self.convertor.Sha256Value(json_UploadMessage) 
        encodeMsg = self.convertor.EncodeRSAtoBase64(shaValue)
        
        # dbus发送
        try:
            self.sender.MsgSendToServer(json_UploadMessage, json_PackageInfo, encodeMsg)
        except Exception as e:
            logging.error(e)

    def Generate_Msg(self, upgrade_list, mode):
        try:
            self.upgrade_list = upgrade_list
            self.upgrade_mode = mode
            self.UpdateInfos.update({"upgradeMode":self.mode_map.get(self.upgrade_mode, "default-mode")})
        except DBusException as e:
            logging.error(e)

    def Upgrade_Process_Msg(self, action, dict_msg = {}):
        if self.updateManager.configs_uncover.getWithDefault("SystemStatus", "upload_upgrade_log", False) == True:
            tmp_dict = {}
            tmp_dict.update(dict_msg)
            try:
                self.Init_UUID()
                if 'error_string' in tmp_dict.keys() and 'error_desc' in tmp_dict.keys():
                    if tmp_dict['error_string'] == "下载失败" and tmp_dict['error_desc'] == "取消升级":
                        tmp_dict.update({"error_string":str(tmp_dict['error_string']+","+tmp_dict['error_desc'])})
                        tmp_dict.update({"error_desc":str(tmp_dict['error_string']+","+tmp_dict['error_desc'])})
                self.UpdateInfos.update({"step":self.action_map.get(action, "")})
                if self.upgrade_mode == self.MODE_INSTALL_SYSTEM:
                    self.UpdateInfos.update({"appname":"Upgrade System"})
                    tmp_dict.update(self.UpdateInfos)
                    json_file = json.dumps(tmp_dict.copy())
                    self.UpdateMsg(self.messageType_map.get(action, ""), json_file, self.uuid)
                else:
                    if action == self.ACTION_INSTALL_DEB:
                        tmp_dict.update({"step":self.action_map.get(action, "")})
                        json_file = json.dumps(tmp_dict.copy())
                        self.UpdateMsg(self.messageType_map.get(action, ""), json_file, self.uuid)
                    else:
                        tmp_dict.update(self.UpdateInfos)
                        json_file = json.dumps(tmp_dict.copy())
                        self.UpdateMsg(self.messageType_map.get(action, ""), json_file, self.uuid)
            except Exception as e:
                logging.error(e)
            tmp_dict.clear()

    def make_background_version(self,pkg):
        if pkg.is_installed == True:
            self.background_version.update({pkg.name:pkg.installed.source_version})
        else:
            self.background_version.update({pkg.name:"Unknown"})
        self.background_list.append(pkg.name)
    
    def Init_UUID(self):
        if self.uuid == "":
            self.uuid = str(uuid.uuid1())
    
    def Msg_Clean(self):
        self.UploadMessage = {}
        self.PackageInfo = {}
        self.UpdateInfos = {}
        self.uuid = ""

class FormatConvert():
    def __init__(self, DataCollector):
        #秘钥
        self.publickey = UniqueKey()
        self.collector = DataCollector

    def dictConvertJson(self, dict_msg):
        #字典转换为json格式字符串
        json_file = ''
        try:
            json_file = json.dumps(dict_msg)
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


class MessageSend():
    ERR_PARA_FROMAT = 1
    ERR_NO_LOACLTID = 2
    ERR_ABNORMAL_SHA = 3
    ERR_UPLOADMSG_SHA = 4
    ERR_UPLOADMSG_CTS = 5

    def __init__(self, DataCollector=None) -> None:
        # self.convertor = FormatConvert()
        if DataCollector == None:
            self.collector = UpdateMsgCollector()
        else:
            self.collector = DataCollector

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
        self.Send_finally(retval, retid, PackageInfo, UploadMessage, encodeMsg)

    def Send_finally(self, retval, retid, json_PackageInfo, json_UploadMessage, encodeMsg):
        # 根据发送结果进行处理
        result = ''
        PackageInfo = self.collector.convertor.JsonConvertDict(json_PackageInfo)
        if retval != 0: 
            if retval == self.ERR_PARA_FROMAT:
                result = "Parameter format error"
                logging.error("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
            elif retval == self.ERR_NO_LOACLTID:
                result = "The tid value in packageInfo is abnormal, but the message is saved successfully"
                logging.info("Sent Status: false - packageName: %s : result: %s.", PackageInfo['packageName'], result)
                # 将返回的tid保存到本地
                key = PackageInfo['packageName']+'_'+PackageInfo['messageType']
                self.SaveTid(key, retid)
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
            # 上传失败写入本地json
            if retval != self.ERR_NO_LOACLTID or retval == self.ERR_NO_LOACLTID:
                self.WriteToJson(PackageInfo['messageType'], json_PackageInfo, json_UploadMessage, encodeMsg)
        elif retval == 0:
            result = "Send to server success"
            logging.info("Sent Status: True - packageName: %s : result: %s.", PackageInfo['packageName'], result)

    def GetLocalTid(self, key):
        # 试图获取本地tid
        try:
            # 存放至数据库
            tid = self.collector.updateManager.sqlite3_server.select_from_tid("tid",key)
            if tid == "None" or tid == None:
                self.localtid = ""
            else:
                self.localtid = tid
        except Exception as e:
            logging.error(str(e))

    def SaveTid(self, key, localtid):
        if len(localtid) == 0:
            return 
        _localtid = str(localtid)
        try:
            # 写入数据库
            self.collector.updateManager.sqlite3_server.insert_into_tid(key, _localtid)
        except Exception as e:
            logging.error(str(e))

    def WriteToJson(self, messageType, json_PackageInfo, json_UploadMessage, encodeMsg):
    #发送失败时，写入本地json中定时发送
        Msg = {}
        Msg["PackageInfo"] = json_PackageInfo
        Msg["UploadMessage"] = json_UploadMessage
        Msg["encodeMsg"] = str(encodeMsg)
        json_file = self.collector.convertor.dictConvertJson(Msg)
        # 保存信息
        try: 
            if not os.path.exists(MSGSNDDIR):
                os.mkdir(MSGSNDDIR)
            # 根据messageType保存信息
            with open(MSGSNDDIR+messageType+".json","a") as f:
                f.write(json_file)
                f.write("\n")
        except Exception as e:
            logging.error(str(e))

    def _TimedTransmission(self, file_path = MSGSNDDIR):
        classify_list = [name for name in os.listdir(file_path) if name.endswith(".json")]
        for f in classify_list:
            # 循环发送每一个文件
            self._ReadFromFile(os.path.join(file_path, f))
    
    def _ReadFromFile(self, json_path):
        new_lines = []
        # 从本地文件读取
        if not os.path.exists(json_path):
            return 
        with open(json_path, "r+") as f:
            lines = f.readlines()

        # file is empty and path is exit -> remove file
        if len(lines) == 0 and os.path.exists(json_path):
            os.remove(json_path)
            return 

        #send installinfo or updateinfo
        for line in lines:
            (retval,retid) = self._file_send_server(line) 
            if retval != 0: # success
                new_lines.append(line)
        if os.path.exists(json_path):
            os.remove(json_path)
        if len(new_lines) != 0:
            with open(json_path, "w+") as f:
                for line in lines:
                    f.write(line)

    def _file_send_server(self, json):
        UploadMessage = {}
        PackageInfo   = {}
        encodeMsg     = ''
        dict_msg = self.collector.convertor.JsonConvertDict(json)
        if 'UploadMessage' in dict_msg.keys():
            UploadMessage = dict_msg['UploadMessage']
            UploadMessage = self.collector.convertor.dictConvertJson(UploadMessage)
        if 'PackageInfo' in dict_msg.keys():
            PackageInfo = dict_msg['PackageInfo']
            PackageInfo = self.collector.convertor.dictConvertJson(PackageInfo)
        if 'encodeMsg' in dict_msg.keys():
            encodeMsg = str(dict_msg['encodeMsg'])
        if len(UploadMessage) == 0 or len(PackageInfo) == 0 or encodeMsg == '':
            logging.error("Msg error")
            return 6, ''
        daqbus = dbus.SystemBus()
        try:
            daqobj = daqbus.get_object('com.kylin.daq', '/com/kylin/daq')
            daqinterface = dbus.Interface(daqobj, dbus_interface='com.kylin.daq.interface')
        except DBusException as e:
            logging.error(str(e))
        try:
            retval,retid = daqinterface.UploadMessage(PackageInfo, UploadMessage, encodeMsg)
        except AttributeError:
            logging.error("Call UploadMessage: Attribute Error.")
        return (retval,retid)

class UniqueKey():
    keyvalue = "LS0tLS1CRUdJTiBQVUJMSUMgS0VZLS0tLS0KTUlJQklqQU5CZ2txaGtpRzl3MEJBUUVGQUFPQ0FR\
OEFNSUlCQ2dLQ0FRRUFzdW1NTFJEdlFNb0tEQkRJODRqSgpqc1A0Mk55V0pWVEZob2Jra3ZiT05j\
dExYTXVzRmo2TzJUblZYU3Z6VlVLSjRqZkpwT2l2WEphOVB5Z2wzYTRnClBzSU40enNCMEdOY0tr\
R3VsS2RrV2x6S3lWQ2xlTzhiQnN6SjkwbTc3cWF6YWg3a1A0TUl0WTVFczBpSkpiR0oKN1MxcERj\
MlJkNnVFQWJLaXJyRTFlNzlFTEd4am5VN2V5NWkyRDE2WWJoZEQwZ2lNa2RHR3piQXBKTWZWRVJR\
TQo1NXorMFVqdS8zSFJhNFY3b3p2TGRPRE5HUURaeWNJU0l3VHBLbFR3RjBxazdCNjVhTUlJenQ1\
dnhOK1lxYU1GClppZFRLNzcxNjdqNEExZ3F3MG45bjlybWVXUGRWZ3dudnRtVXp4Q1krNk05SXpK\
TDI3eWpRUTV1WGQ3RVdMT3IKbndJREFRQUIKLS0tLS1FTkQgUFVCTElDIEtFWS0tLS0tCg=="

class PHPServer(threading.Thread):
    LOG_PATH = "/var/log/kylin-system-updater"
    KYLIN_SOFTWARE_PROPERTIES_LOG = "/var/log/kylin-software-properties.log"
    PINGBACK_INTERNET_URL = "http://archive1.kylinos.cn:32294/kylin-update-manager-server/main.php?"
    PINGBACK_INTERNET_FILE_URL = "http://archive1.kylinos.cn:32294/kylin-update-manager-server/get_file.php?"
    PINGBACK_INTRANET_URL = "http://archive.kylinos-intranet.cn/kylin-update-manager-server/main.php?"
    PINGBACK_INTRANET_FILE_URL = "http://archive.kylinos-intranet.cn/kylin-update-manager-server/get_file.php?"
    SYSTEM_VERSION_PATH = "/etc/kylin-version/kylin-system-version.conf"

    def get_values(self, _appname="", _appversion="", _state='', _errorcode='', _errorstring=""):
        self.appname = _appname
        self.appversion = _appversion
        self.status = _state
        self.errorcode = _errorcode
        self.errorstring = _errorstring

    def run(self):
        # 获取本机ip
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            host_ip = s.getsockname()[0]
        except:
            host_ip = 'Ip failed to get'
        # 获取系统版本
        with open('/etc/lsb-release', 'r') as f:
            for line in f.readlines():
                if line.strip().startswith('DISTRIB_DESCRIPTION='):
                    versions = line.strip().split('=')
                    if "V10" in line and "SP1" in versions[1]:
                        version = "V10SP1"
                    else:
                        version = "V10Pro"
                    break
        # 获取软件版本
        output = os.popen('dpkg -l|grep kylin-system-updater').readlines()
        if output:
            soft_version = output[0].strip().split()[2]
        # 获取时间
        nowtime = time.strftime('%Y-%m-%d-%H:%M:%S', time.localtime(time.time()))
        # 获取Mac
        mac = uuid.UUID(int=uuid.getnode()).hex[-12:]
        host_mac = ":".join([mac[e:e + 2] for e in range(0, 11, 2)])
        # 获取序列号
        if os.path.exists("/etc/.kyinfo"):
            kyinfo_configs = UpgradeConfig(datadir = "/etc/", name = ".kyinfo")
            key = kyinfo_configs.getWithDefault("servicekey", "key", "0")
        else:
            key = "0"
        try:
            # 用于收集源管理器的更新日志
            if self.status != "success":
                # nowtime = datetime.utcnow( ).strftime ( '%Y-%m-%d %H:%M:%S.%f' )[:-3]
                nowtime = get_east_8_time()
                log_dir = os.path.join(self.LOG_PATH, host_mac + "_" + nowtime)
                log_file_gzip = log_dir + ".tar.gz"
                os.makedirs(log_dir, exist_ok=True)

                #get updater log
                updater_path = os.path.join(log_dir,"kylin-system-updater")
                os.makedirs(updater_path, exist_ok=True)
                #get updater log 
                if os.path.exists("/var/log/kylin-system-updater/kylin-system-updater.log.1"):
                    shutil.copy("/var/log/kylin-system-updater/kylin-system-updater.log.1", log_dir)
                if os.path.exists("/var/log/kylin-system-updater/kylin-system-updater.log.1.1.gz"):
                    shutil.copy("/var/log/kylin-system-updater/kylin-system-updater.log.1.1.gz", log_dir)

                #get apt log
                if os.path.exists("/var/log/apt/history.log"):
                    shutil.copy("/var/log/apt/history.log", log_dir)
                if os.path.exists("/var/log/apt/term.log"):
                    shutil.copy("/var/log/apt/term.log", log_dir)

                #get version file
                if os.path.exists(self.SYSTEM_VERSION_PATH):
                    shutil.copy(self.SYSTEM_VERSION_PATH, log_dir)
                gZipFile(log_dir, log_file_gzip)
                header = {'Content-Type': "multipart/form-data", "Accept-Encoding": "gzip"}
                try:
                    with open(log_file_gzip, "rb") as f:
                        requests.post(self.PINGBACK_INTRANET_FILE_URL + "filename=" + os.path.basename(log_file_gzip),
                                      data=f.read(), headers=header)
                except:
                    with open(log_file_gzip, "rb") as f:
                        requests.post(self.PINGBACK_INTERNET_FILE_URL + "filename=" + os.path.basename(log_file_gzip),
                                      data=f.read(), headers=header)
                shutil.rmtree(log_dir)
                os.remove(log_file_gzip)
            else:
                log_file_gzip = ""
            kmg_tmp = {'ip': host_ip, 'version': version, 'soft_version': soft_version, 'datetime': nowtime,
                 'host_mac': host_mac, 'appname': self.appname, 'appversion': self.appversion, 'serial_number': key,
                 'state': self.status, 'filename': log_file_gzip, 'errorcode': self.errorcode, 'errorstring': self.errorstring}
            kmg = parse.urlencode(kmg_tmp)
            logging.debug("PHPServer UpdateInfos: %s .", kmg_tmp)
            # 优先使用内网服务器，再使用外网
            try:
                url = self.PINGBACK_INTRANET_URL + kmg
                req = request.urlopen(url=url, timeout=3)
                logging.info("The Intranet log server is successfully accessed, pkgname:%s .",self.appname)
            except:
                url = self.PINGBACK_INTERNET_URL + kmg
                req = request.urlopen(url=url, timeout=3)
                logging.info("The external log server is successfully accessed, pkgname:%s .",self.appname)
        except Exception as e:
            logging.error("Failed to access the external log server: %s, pkgname:%s .", e, self.appname)
            if os.path.isfile(log_file_gzip):
                os.remove(log_file_gzip)

def PHPSeverSend(_appname="", _appversion="", _statue="", _errorcode="", _errorstring=""):
        send_thread = PHPServer()
        send_thread.get_values(_appname=_appname, _appversion=_appversion, _state=_statue, _errorcode=_errorcode, _errorstring=_errorstring)
        send_thread.start()

def gZipFile(src, dst):
  with tarfile.open(dst, "w:gz") as tar:
      tar.add(src, arcname=os.path.basename(src))

def get_east_8_time():
    import time
    # UTC时间
    utc_time = datetime.utcnow() 
    # 转时间字符串
    utc_time = utc_time.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
    time_suffix = utc_time.split(".")[1]
    # 字符串转时间元祖
    utc_time = time.strptime(utc_time, "%Y-%m-%d %H:%M:%S.%f")
    # 时间元祖转时间戳
    utc_time = time.mktime(utc_time)
    # 生成东八区时间时间戳
    now_time = utc_time + 8*60*60
    # 时间戳转时间元祖
    now_time = time.localtime(now_time)
    # 时间元祖转字符串
    now_time = time.strftime("%Y-%m-%d %H:%M:%S",now_time)
    now_time = now_time + "." +time_suffix
    return now_time
    # return 0

if __name__ == "__main__":
    # 执行定时发送
    ms = MessageSend()
    ms._ReadFromFile("/var/lib/kylin-system-updater/sendinfos/testMsg.json")