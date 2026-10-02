#!/usr/bin/python3
# 
#  libkysdk-system's Library
#  
#  Copyright (C) 2023, KylinSoft Co., Ltd.
# 
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this library.  If not, see <https://www.gnu.org/licenses/>.
# 
#  Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
# 
# 

import os
import logging
import dbus
import dbus.service
from dbus.mainloop.glib import DBusGMainLoop
from gi.repository import GLib
import conf2Utils
import subprocess

logger = None

# 创建一个DBus服务类
class KYSettingsDBusService(dbus.service.Object):
    def __init__(self, bus_name, object_path):
        logger.info(f'DBus Class init')
        dbus.service.Object.__init__(self, bus_name, object_path)

        self._converter = conf2Utils.converter(logger)
        self._watchdog = conf2Utils.FileWatche('/etc/kylin-config/basic', self)

        if not os.path.exists("/etc/kylin-config/user.db"):
            self._converter.update_read_only_db()
        self._converter.update_user_db("/root/.config/kylin-config/user.db")

        logger.info(f'DBus Class init end')

    # 重载
    def _reload(self):
        result = subprocess.run(['/usr/bin/conf2-compile'], capture_output=True, text=True)
        print(result.stdout)
        if len(result.stdout) == 0:
            if self._converter.update_user_db('/root/.config/kylin-config/user.db'):
                return True
        return False

    # key值改变信号
    @dbus.service.signal("com.kylin.kysdk.conf2", signature='sss')
    def key_changed(self, id, version, key):
        pass

    # 配置信息update信号
    @dbus.service.signal("com.kylin.kysdk.conf2", signature='')
    def updated(self):
        pass

    # 设置一个配置的值
    @dbus.service.method("com.kylin.kysdk.conf2", in_signature='sssss', out_signature='i', sender_keyword='sender')
    def set(self, user, id, version, key, value, sender = None):
        logger.info(f'set begin')
        success = False
        if user == 'root':
            success = self._converter.set('/root/.config/kylin-config/user.db',  id, version, key, value)
        else:
            success = self._converter.set(f'/home/{user}/.config/kylin-config/user.db', id, version, key, value)

        if success:
            self.key_changed(id, version, key)
            logger.info(f'set success')
        else:
            logger.error(f'set failed')

        return success
    
    # 恢复单个默认值
    @dbus.service.method("com.kylin.kysdk.conf2", in_signature='ssss', out_signature='i', sender_keyword='sender')
    def reset(self,user, id, version, key, sender = None):
        logger.info(f'reset begin')
        success = False
        if user == 'root':
            success = self._converter.reset('/root/.config/kylin-config/user.db',  id, version, key)
        else:
            success = self._converter.reset(f'/home/{user}/.config/kylin-config/user.db',  id, version, key)

        if success:
            self.key_changed(id, version, key)
            logger.info(f'reset success')
        else:
            logger.error(f'reset failed')
        
        return success

    # 恢复整个id的默认值
    @dbus.service.method("com.kylin.kysdk.conf2", in_signature='', out_signature='i', sender_keyword='sender')
    def reload(self, sender = None):
        logger.info(f'reload begin')
        success = self._reload()
        if success:
            self.updated()
            logger.info(f'reload success')
        else:
            logger.error(f'reload failed')
        return success

    # 导出配置数据到文件
    @dbus.service.method("com.kylin.kysdk.conf2", in_signature='ss', out_signature='b', sender_keyword='sender')
    def save(self, user, path, sender = None):
        logger.info(f'save begin')
        success = False
        if user == 'root':
            success = self._converter.save('/root/.config/kylin-config/user.db', path)
        else:
            success = self._converter.save(f'/home/{user}/.config/kylin-config/user.db', path)

        if success:
            self.updated()
            logger.info(f'save success')
        else:
            logger.error(f'save failed')
        
        return success

    #复制old_id的配置到new_id的位置，并将新的数据写入到数据库
    @dbus.service.method("com.kylin.kysdk.conf2", in_signature='ssss', out_signature='b', sender_keyword='sender')
    def extends_id(self, old_id:str, new_id:str, version:str, user:str, sender = None):
        logger.info(f'extends_id begin')
        success = False
        if user == 'root':
            success = self._converter.extends_id(old_id,  new_id, version, '/root/.config/kylin-config/user.db')
        else:
            success = self._converter.extends_id(old_id,  new_id, version, f'/home/{user}/.config/kylin-config/user.db')

        if success:
            self.updated()
            logger.info(f'extends_id success')
        else:
            logger.error(f'extends_id failed')
        return success

if __name__ == "__main__":
    os.makedirs('/etc/kylin-config', exist_ok=True)
    logging.basicConfig(filename='/etc/kylin-config/systembus.log', 
                        level=logging.DEBUG, 
                        format='%(asctime)s.%(msecs)03d - %(name)s - %(levelname)s - %(message)s', 
                        datefmt='%Y-%m-%d %H:%M:%S')
    logger = logging.getLogger()

    # 初始化DBus主循环
    DBusGMainLoop(set_as_default=True)

    # 连接到DBus系统总线
    bus = dbus.SystemBus()

    # 注册一个DBus服务
    bus_name = dbus.service.BusName("com.kylin.kysdk.conf2", bus=bus)
    object_path = "/com/kylin/kysdk/conf2"
    my_dbus_service = KYSettingsDBusService(bus_name, object_path)

    # 开始DBus主循环
    mainloop = GLib.MainLoop()
    mainloop.run()
