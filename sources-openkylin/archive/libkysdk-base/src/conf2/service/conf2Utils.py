# /*
#  * Copyright (C) 2024-2025, KylinSoft Co., Ltd.
#  *
#  * This library is free software; you can redistribute it and/or
#  * modify it under the terms of the GNU Lesser General Public
#  * License as published by the Free Software Foundation; either
#  * version 3 of the License, or (at your option) any later version.
#  *
#  * This library is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  * Lesser General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this library.  If not, see <https://www.gnu.org/licenses/>.
#  *
#  * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
#  *
#  */
import os
import re
import shutil
import yaml
import sqlite3
import logging
import traceback
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import subprocess

# 这部分主要面向chatgpt编程，实在有看不懂的请面向chatgpt提问

class converter:
    '''通用类，维护统一视图/用户视图'''
    def __init__(self, logger:logging.Logger):
        self.logger = logger
        self._ro_db = '/etc/kylin-config/user.db'
        self.SqlHelper = sqlHelper(logger)
        self.YamlHelper = yamlHelper(logger)
        self._user_dict = dictHelper()

    def update_read_only_db(self):
        """更新etc下数据库\n

        兼容旧版本的数据库，创建新版数据库并迁移数据\n
        读取/etc下所有配置并合并覆盖\n
        把数据写到etc的数据库中
        """
        try:
            if not self.SqlHelper.update_table_structure(self._ro_db):
                self.SqlHelper.creat_db_file(self._ro_db)
            ro_dict = self.YamlHelper.read_yaml_dirs()
            return self.SqlHelper.dict2db(ro_dict, self._ro_db)
        except:
            err_msg = traceback.format_exc()
            self.logger.error(err_msg)
            return False
        
    def update_user_db(self, path):
        """更新用户数据库
        
        兼容旧版本的数据库，创建新版数据库并迁移数据
        合并/复制etc下数据库数据到用户数据库
        """
        try:
            if not os.path.exists(self._ro_db):
                self.logger.info('/etc/kylin-config/user.db not existed')
                return False
            
            existed = self.SqlHelper.update_table_structure(path)
            if not existed:
                os.makedirs(os.path.dirname(path), exist_ok=True)
                shutil.copyfile(self._ro_db, path)
                self.update_user_dict(path)
            else:
                if self._user_dict == {}:
                    self.update_user_dict(path)

                data = self.SqlHelper.db2dict(self._ro_db)
                self._user_dict.del_discarded_key(data)
                self._user_dict.update_recursively(data, 0)

            # 同步gsetting的用户设置到conf2。在应用新装包时，如果存在同名gsetting的id，会生成map.yaml文件
            self._set_gsettings_config_in_conf2(path)

            # 如果是root用户且第一次创建用户数据库， 至此流程结束
            if path.startswith('/root') and not existed:
                return True
            
            # 加载用户目录配置文件，路径在~/.config/kylin-config/configs/
            # 等同于/etc下的配置文件，但仅在当前用户合并
            home_path = os.getenv('HOME')
            usr_configs = f'{home_path}/.config/kylin-config/configs'
            if path.startswith('/home') and os.path.exists(usr_configs):
                usr_data = self.YamlHelper.read_yaml_dir(usr_configs)
                self._user_dict.update_recursively(usr_data, 2)

            self.SqlHelper.dict2db(self._user_dict, path)
            return True
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(err_msg)
            return False

    def update_user_dict(self, path):
        self._user_dict = self.SqlHelper.db2dict(path)
    
    def set(self, path, id, version, key, value):
        try:
            # 修改用户配置文件，写入到数据库
            group_list = id.split('.')

            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            # 开始写入事务
            conn.execute('BEGIN TRANSACTION')

            cursor.execute("SELECT id FROM app WHERE app_name = ?", (group_list[0],))
            app_id = cursor.fetchone()[0]

            cursor.execute("SELECT id FROM version WHERE app_id = ? AND version = ?", (app_id, version))
            version_id = cursor.fetchone()[0]

            # 查找组
            parent_id = 0
            for group_name in group_list[1:]:
                cursor.execute("SELECT id FROM configures WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'schema'", (version_id, parent_id, group_name))
                parent_id = cursor.fetchone()[0]

            cursor.execute("UPDATE configures SET custom_value = ? WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'key'", (value, version_id, parent_id, key))

            # 修改缓存
            tmp = self._user_dict[group_list[0]][version]
            for group in group_list[1:]:
                tmp = tmp[group]
            tmp[key]['_value'] = value

            conn.commit()
            conn.close()

            return True
        
        except Exception as e:
            conn.rollback()
            conn.close()
            err_msg = traceback.format_exc()
            self.logger.error(f'set {err_msg}')
            return False
    
    # 重置配置值
    def reset(self, path, id, version, key):
        try:
            # 修改用户配置文件，写入到数据库
            group_list = id.split('.')

            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            # 开始写入事务
            conn.execute('BEGIN TRANSACTION')

            cursor.execute("SELECT id FROM app WHERE app_name = ?", (group_list[0],))
            app_id = cursor.fetchone()[0]

            cursor.execute("SELECT id FROM version WHERE app_id = ? AND version = ?", (app_id,version))
            version_id = cursor.fetchone()[0]

            # 查找组
            parent_id = 0
            for group_name in group_list[1:]:
                cursor.execute("SELECT id FROM configures WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'schema'", (version_id, parent_id, group_name))
                parent_id = cursor.fetchone()[0]

            cursor.execute("UPDATE configures SET custom_value = NULL WHERE version_id = ? AND parent = ? AND node_name = ? AND node_type = 'key'", (version_id, parent_id, key))

            tmp = {}
            tmp = self._user_dict[group_list[0]][version]
            for group in group_list[1:]:
                tmp = tmp[group]
            if '_value'  in tmp[key]:
                tmp[key].pop('_value')

            conn.commit()
            conn.close()

            return True
        
        except Exception as e:
            conn.rollback()
            conn.close()
            err_msg = traceback.format_exc()
            self.logger.error(f'set {err_msg}')
            return False

    # 配置数据导出到文件
    def save(self, db_file:str, dest_path:str):
        try:
            if dest_path.startswith('/etc'):
                if dest_path != '/etc/kylin-config/basic':
                    return False
            else:
                if not dest_path.startswith('/home'):
                    return False

            if not os.path.exists(dest_path):
                self.logger.error(f'{dest_path} not exists')
                return False
            
            data = self.SqlHelper.db2dict(db_file)

            if data == {}:
                self.logger.error(f'read sqlite failed')
                return False
            
            #将用户配置写入到复制的只读视图里
            self._value_override_default(data)

            for key in data:
                with open(f'{dest_path}/{key}.yaml', 'w') as yaml_file:
                    yaml_file.write(yaml.safe_dump({key:data[key]}, allow_unicode = True))

            return True
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'save {err_msg}')
            return False

    # 用于kconf2-editor保存文件，仅有普通用户权限，root用户没有图形化界面 
    def editor_save(self, apps:list, data, dest_path):
        try:
            self.logger.info(f'editor_save begin')
            
            #将用户配置写入到复制的只读视图里
            self._value_override_default(data)

            for app in apps:
                with open(f'{dest_path}/{app}.yaml', 'w') as yaml_file:
                    yaml_file.write(yaml.safe_dump({app:data[app]}, allow_unicode = True))

            self.logger.info(f'editor_save end')
            return True
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'editor_save {err_msg}')
            return False

    # 创建新id内容继承自旧id。对应gsettings重定向
    def extends_id(self, old_id:str, new_id:str, version:str, target_file):
        try:
            target_view = self.SqlHelper.db2dict(target_file)
            
            #获取app的引用
            list = old_id.split('.', -1)
            app = target_view.get(list[0])

            if app is None:
                self.logger.error(f'{old_id} is not exists')
                return False
            
            #获取目标版本的引用，如果为空，获取默认版本
            if version == '':
                old_data = app[app['_default_version']]
            else:
                old_data = app.get(version)

            if old_data is None:
                self.logger.error(f'{version} is not invalid')

            for iter in list[1:]:
                old_data = old_data.get(iter)
                if old_data is None:
                    self.logger.error(f'{old_id} is not exists')
                    return False
                
            tmp = {}
            tmp.update(old_data)

            list = new_id.split('.', -1)
            new_data = target_view.get(list[0])
            if new_data != app:
                self.logger.error(f'{new_id} is invalid')

            if version == '':
                new_data = app[app['_default_version']]
            else:
                new_data = app.get(version)
            
            for iter in list[1:]:
                if iter not in new_data:
                    new_data[iter] = {}
                new_data = new_data[iter]
            
            new_data.update(tmp)
            new_data['_extends'] = old_id
            return self.SqlHelper.dict2db(target_view, target_file)
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'extends_id {err_msg}')
            return False

    # 导出时，使用value的值替带default值
    def _value_override_default(self, data:dict):
        for key, value in data.items():
            if isinstance(value, dict):
                self._value_override_default(value)
            else:
                if '_value' in data:
                    data['_default'] = data.pop('_value')
                return

    # 将gsettings的用户配置写入到conf2数据库
    def _gsettings_config_get(self, data, id):
        for key, value in data.items():
            if not isinstance(value, dict):
                continue
            if '_default' in value:
                result = subprocess.run(['gsettings','get', id, key], capture_output=True, text=True)
                custom_value =  result.stdout.strip()
                custom_value = custom_value.split(' ')[-1] if custom_value.startswith('@') else custom_value
                if custom_value:
                    value['_value'] = custom_value
            else:
                self._gsettings_config_get(value, f'{id}.{key}')

    def _set_gsettings_config_in_conf2(self, path):
        if os.path.exists(f'{os.path.dirname(path)}/map.yaml'):
            self.logger.info("override gsettings config")
            with open(f'{os.path.dirname(path)}/map.yaml', 'r') as file:
                mmap = yaml.safe_load(file)
            if mmap is not None:
                for app, gsetting_id in mmap.items():
                    for version, version_dict in self._user_dict[app].items():
                        if not isinstance(version_dict, dict):
                            continue
                        self._gsettings_config_get(self._user_dict[app][version], gsetting_id)
            os.remove(f'{os.path.dirname(path)}/map.yaml')

class FileWatche:
    """文件监听类,监听basci目录
    负责自动reload
    负责新配置文件是否存在gsetting配置数据需要迁移
    """
    class MyHandler(FileSystemEventHandler):
        def __init__(self, server):
            self._server = server

        def on_modified(self, event):
            if event.is_directory:
                return
            
            if not event.src_path.endswith('.yaml'):
                return
            
            print(f'{event.event_type} -> {event.src_path}')
            self._server.reload()

        def on_created(self, event):
            if event.is_directory:
                return
            
            if not event.src_path.endswith('.yaml'):
                return
            
            print(f'{event.event_type} -> {event.src_path}')
            gsettings_id = os.path.basename(event.src_path).strip(".yaml")
            gsettings_schemas = subprocess.run(['gsettings','list-schemas'], capture_output=True, text=True)
            if gsettings_id in gsettings_schemas.stdout:
                mmap = {}
                with open(event.src_path, 'r') as file:
                    data = yaml.safe_load(file)

                for app, version in data.items():
                    if not isinstance(version, dict):
                        continue
                    mmap[app] = gsettings_id

                home_directories = [os.path.expanduser('~' + username) for username in os.listdir('/home')]
                home_directories.append('/root')
                for home_dir in home_directories:
                    file_path = os.path.join(home_dir, '.config/kylin-config/map.yaml')
                    if not os.path.exists(os.path.dirname(file_path)):
                        continue

                    if os.path.exists(file_path):
                        with open(file_path, 'r') as file:
                            tmp = yaml.safe_load(file)
                            if tmp is not None:
                                mmap.update(tmp)

                        os.remove(file_path)

                    with open(file_path, 'w') as file:
                        file.write(yaml.safe_dump(mmap))

                    os.chmod(file_path, 0o666)
            
    def __init__(self, path, server) -> None:
        self._server = server
        self._observer = Observer()
        self._event_handler = FileWatche.MyHandler(server)

        self._observer.schedule(self._event_handler, path, recursive=False)
        self._observer.start()

    def stopWatche(self):
        if self._observer.isAlive():
            self._observer.stop()
            self._observer.join()

class dictHelper(dict):
    '''字典工具类
    
    负责字典数据操作，如合并，覆盖
    '''
    def __new__(cls, *args, **kwargs):  
        # 使用 super() 调用 dict 的 __new__ 方法  
        if len(args) == 1 and isinstance(args[0], dict):  
            # 返回对传入字典的引用，而不是副本  
            return super(dictHelper, cls).__new__(cls, args[0])  
        return super(dictHelper, cls).__new__(cls)  

    def __init__(self, *args, **kwargs):  
        # 调用父类的初始化方法  
        if len(args) == 1 and isinstance(args[0], dict):  
            # 如果是通过字典初始化，直接指向这个字典  
            self.update(args[0])  # Update current instance with the passed dictionary  
        else:  
            super(dictHelper, self).__init__(*args, **kwargs)  

    def __repr__(self):  
        return f'dictHelper({super().__repr__()})'

    def copy(self):  
        # 重写 copy 方法返回 dictHelper 类型  
        return dictHelper(super().copy())  

    def update(self, *args, **kwargs):  
        # 重写 update 方法  
        super().update(*args, **kwargs)
    
    def __getitem__(self, key): 
        # []获取键值的实现 
        value = super().__getitem__(key)  
        return value  

    def __delitem__(self, key):  
        # 删除当前键  
        super().__delitem__(key)

    @classmethod  
    def fromkeys(cls, iterable, value=None):  
        # 重写 fromkeys 方法  
        return cls(dict.fromkeys(iterable, value)) 

    def _update_recursively(self, dest:dict, src:dict, type:int, pass_type_changed:bool):
        if src == {}:
            return
        
        if dest == {}:
            dest.update(src)
            return
        
        if '_permission' in dest:
            if type == 1:
                if dest['_permission'] == 'final':
                    return
            elif type == 2:
                if dest['_permission'] == 'final' or dest['_permission'] == 'stable':
                    return
        if pass_type_changed and '_type' in dest:
            if dest['_type'] != src['_type']:
                return
        
        for key in src.keys():
            if key in dest :
                if isinstance(dest[key], dict) and isinstance(src[key], dict):
                    self._update_recursively(dest[key], src[key], type, pass_type_changed)
                else:
                    dest[key]  = src[key]
            else:
                dest[key] = src[key]
        return

    def update_recursively(self, src:dict, type:int=0, pass_type_changed:bool = False):
        '''
        将源字典递归合并\n
        src 源字典\n
        type 0 完全合并 1 final权限的配置不再覆盖 2 final和stable权限的配置不再覆盖\n
        pass_type_changed 跳过数据类型改变的键
        '''
        self._update_recursively(self, src, type, pass_type_changed)

    def _del_discarded_key(self, dest:dict, src:dict):
        keys_to_delete = []
        for key in dest:
            if not isinstance(dest[key], dict):
                continue
            if key not in src:
                if dest[key].get('_extends') is None:
                    keys_to_delete.append(key)
            else:
                self._del_discarded_key(dest[key], src[key])

        for key in keys_to_delete:
            dest.pop(key)

    def del_discarded_key(self, src:dict):
        '''删除自身存在, 但src中不存在的键'''
        self._del_discarded_key(self, src)

    def custom_configuration_merge_versions(self):
        '''将字典中所有版本配置合并，高版本会覆盖低版本''' 
        # 创建 CustomVersion 对象并排序  
        for app in self:
            app_data = self[app]

            # 跳过通用组配置
            is_group_file = True if '_default' in next(iter(app_data.values())) else False
            if is_group_file:
                continue

            versions = app_data.keys()
            custom_versions = [CustomVersion(v) for v in versions]
            sorted_versions = sorted(custom_versions)

            if len(sorted_versions) > 1:
                version_data = dictHelper(app_data[sorted_versions[0].version_str])
                for version in sorted_versions[1:]:
                    version_data.update_recursively(app_data[version.version_str], 0, True)
                for version in sorted_versions[1:]:
                    app_data = self.get(app)
                    del app_data[version.version_str]

    def _group_override_basic(self,group:str, group_data:dict):
        # 通用组配置覆盖
        for app in self:
            for version in self[app]:
                if not isinstance(self[app][version], dict):
                    continue

                if group in self[app][version]:
                    basic_group = dictHelper(self[app][version][group])
                    basic_group.update_recursively(group_data, 1, True)

    def _application_override_basic_without_version(self, app:str, app_data:dict):
        # 应用配置覆盖
        if app in self:
            for version in self[app]:
                if not isinstance(self[app][version], dict):
                    continue

                basic_version_data = dictHelper(self[app][version])
                custom_version_data = app_data[next(iter(app_data))]
                basic_version_data.update_recursively(custom_version_data, 1, True)
        else:
            self[app] = app_data
    
    def override_without_version(self, src:dict):
        '''定制配置覆盖basic配置, 覆盖所有版本'''
        for app, app_data in src.items():
            # 跳过通用组配置
            is_group_file = True if '_default' in next(iter(app_data.values())) else False
            if is_group_file:
                self._group_override_basic(app, app_data)
            else:
                self._application_override_basic_without_version(app, app_data)

    def _application_override_basic_with_version(self, app:str, app_data:dict):
        # 应用配置覆盖
        if app in self:
            basic_version_data = dictHelper(self[app])
            basic_version_data.update_recursively(app_data, 1, True)
            self[app] = basic_version_data
        else:
            self[app] = app_data
    
    def override_with_version(self, src:dict):
        '''定制配置覆盖basic配置, 覆盖对应版本'''
        for app, app_data in src.items():
            # 跳过通用组配置
            is_group_file = True if '_default' in next(iter(app_data.values())) else False
            if is_group_file:
                self._group_override_basic(app, app_data)
            else:
                self._application_override_basic_with_version(app, app_data)

class yamlHelper:
    '''yaml工具类\n

    负责配置文件相关操作
    '''
    def __init__(self, logger:logging.Logger):
        self.logger = logger

    def _read_yaml_dirs_with_version(self, dirs:list=[]):
        try:
            basic_dict = dictHelper()
            if not dirs:
                with open('/etc/kylin-config/conf2.yaml', 'r') as file:
                    configures = yaml.safe_load(file)
                    for dir in configures['dirs']:
                        dirs.append(f'/etc/kylin-config/{dir}')
            if '/etc/kylin-config/basic' in dirs:
                dirs.remove('/etc/kylin-config/basic')
            
            basic_dict = self.read_yaml_dir('/etc/kylin-config/basic')

            for dir in dirs:
                basic_dict.override_with_version(self.read_yaml_dir(dir))

            return basic_dict

        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'load {file} fail: {err_msg}')
            return {}
        
    def _read_yaml_dirs_without_version(self, dirs:list=[]):
        try:
            basic_dict = dictHelper()
            customized_dict = dictHelper()
            if not dirs:
                with open('/etc/kylin-config/conf2.yaml', 'r') as file:
                    configures = yaml.safe_load(file)
                    for dir in configures['dirs']:
                        dirs.append(f'/etc/kylin-config/{dir}')
            if '/etc/kylin-config/basic' in dirs:
                dirs.remove('/etc/kylin-config/basic')
            
            basic_dict = self.read_yaml_dir('/etc/kylin-config/basic')

            for dir in dirs:
                customized_dict.clear()
                customized_dict.update_recursively(self.read_yaml_dir(dir), 1)
            
                # 用定制配置中高版本的覆盖低版本的配置，最后只剩一个版本
                customized_dict.custom_configuration_merge_versions()

                # 弱化定制配置文件的版本作用，将定制配置覆盖到基础配置的所有版本上
                basic_dict.override_without_version(customized_dict)

            return basic_dict

        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'load {file} fail: {err_msg}')
            return {}

    def read_yaml_dirs(self, dirs:list=[]):
        '''读取所有文件夹配置，并完成叠加覆盖'''
        # return self._read_yaml_dirs_with_version(dirs)
        return self._read_yaml_dirs_without_version(dirs)

    def read_yaml_dir(self, path):
        dir_dict = dictHelper()

        files = self._sort_diretory(path)
        for file in files:
            opt_path = path.replace('/etc/kylin-config', '/opt/system/conf/kylin/kylin-config')
            real_path = f'{path}/{file}' if os.path.exists(f'{path}/{file}') else f'{opt_path}/{file}'
            data = self.read_yaml_file(real_path)

            if not data or not isinstance(data, dict):
                continue

            dir_dict.update_recursively(data, 1)
        return dir_dict

    def read_yaml_file(self, file:str):
        try:
            chinese_pattern = re.compile(u'[\u4e00-\u9fa5]+')
            if bool(chinese_pattern.search(file)) or file.startswith('.'):
                return {}

            yaml_data = dictHelper()
            if file.endswith('.yaml'):
                with open(file,'r') as stream:
                    yaml_data = yaml.safe_load(stream)
                if yaml_data is None:
                    self.logger.error(f'Load {file} failed. Check the text formatting')
                    return {}
            return yaml_data
        except Exception:
            err_msg = traceback.format_exc()
            self.logger.error(f'load {file} fail: {err_msg}')
            return {}
    
    def _sort_diretory(self, yaml_dir):
        '''对文件夹内文件进行排序\n

        规则：\n
            不带xxx-前缀的按字母序排列,带xxx-前缀的按xxx大小升序排列
        '''
        if not os.path.exists(yaml_dir):
            return []

        # 获取目录内所有文件名
        # file_list = os.listdir(yaml_dir)
        opt_dir = yaml_dir.replace("/etc/kylin-config", "/opt/system/conf/kylin/kylin-config")
        file_list = os.listdir(yaml_dir)
        if os.path.exists(opt_dir):
            file_list = file_list + os.listdir(opt_dir)

        # 定义排序函数
        def custom_sort(file_name):
            match = re.match(r"(\d+)?-?(.*)", file_name)
            if match.group(1):
                return (int(match.group(1)), match.group(2))
            else:
                return (0, match.group(2))

        sorted_file_list = sorted(file_list, key=custom_sort)
        return sorted_file_list

class CustomVersion:  
    '''版本排序类'''  
    def __init__(self, version_str):  
        self.version_str = version_str  
        self.parts = self.parse_version(version_str)  

    def parse_version(self, version_str):  
        # # 拆分主版本号、次版本号、修订号和附加部分  
        # match = re.match(r"(\d+)\.(\d+)\.(\d+)(?:\.(\d+))?(?:-([A-Za-z0-9.]+))?", version_str)  
        # if not match:  
        #     raise ValueError(f"Invalid version string: {version_str}")  

        # major = int(match.group(1))  
        # minor = int(match.group(2))  
        # patch = int(match.group(3))  
        # sub = int(match.group(4)) if match.group(4) else 0  
        # prerelease = match.group(5) if match.group(5) else ''  

        # # 返回一个可用于比较的元组  
        # return (major, minor, patch, sub, self.split_prerelease(prerelease))  
        nums = re.findall('\d+', version_str)
        segments = re.split('\d+', version_str)
        return (nums, segments)

    def split_prerelease(self, prerelease):  
        # 将预发布标签拆分成多个部分，以便逐个比较  
        return prerelease.split('.') if prerelease else []  

    def __lt__(self, other):  
        return self.compare(self.parts, other.parts)  

    def compare(self, parts1, parts2):  
        # # 比较主版本、次版本、修订号、次版本号  
        # for a, b in zip(parts1[:4], parts2[:4]):  # 仅比较前四个元素  
        #     if a != b:  
        #         return a < b  

        # # 比较预发布标签部分  
        # prerelease1 = parts1[4]  
        # prerelease2 = parts2[4]  

        # # 使用 zip_longest 处理长度不同的预发布部分  
        # from itertools import zip_longest  
        # for part1, part2 in zip_longest(prerelease1, prerelease2, fillvalue=''):  
        #     if part1 == part2:  
        #         continue  
        #     return self.compare_prerelease_component(part1, part2)  

        # # 如果到这里，版本号（主、次和修订）相同，比较预发布  
        # return len(prerelease1) < len(prerelease2)  
        nums1 = parts1[0]
        segments1 = parts1[1]
        nums2 = parts2[0]
        segments2 = parts2[1]
        print((segments1,segments2))
        if segments1 != segments2:
            raise ValueError(f"Invalid version string")
        if len(nums1) != len(nums2):
            raise ValueError(f"Invalid version string")
        for i in range(len(nums1)):
            if int(nums1[i]) != int(nums2[i]):
                return int(nums1[i]) < int(nums2[i])

    def compare_prerelease_component(self, part1, part2):  
        # 比较预发布标签中的单个部分  
        if part1.isdigit() and part2.isdigit():  
            return int(part1) < int(part2)  
        return part1 < part2  # 字符串比较  

    def __repr__(self):  
        return f"CustomVersion({self.version_str})" 

class sqlHelper:
    '''
    数据库工具类

    负责数据库读写、更新
    '''
    def __init__(self, logger:logging.Logger):
        self.logger = logger

    def creat_db_file(self, path):
        '''创建数据库以及表结构'''
        conn = sqlite3.connect(path)
        cursor = conn.cursor()

        conn.execute('BEGIN TRANSACTION')

        cursor.execute('''CREATE TABLE IF NOT EXISTS app
                    (id INTEGER PRIMARY KEY AUTOINCREMENT,
                    app_name TEXT,
                    default_version TEXT)''')
        cursor.execute('''CREATE TABLE IF NOT EXISTS version
                        (id INTEGER PRIMARY KEY AUTOINCREMENT,
                        app_id INTEGER,
                        version TEXT,
                        compatible TEXT,
                        FOREIGN KEY (app_id) REFERENCES app(id))''')
        cursor.execute('''CREATE TABLE IF NOT EXISTS configures
                        (id INTEGER PRIMARY KEY AUTOINCREMENT,
                        version_id INTEGER,
                        node_name TEXT,
                        node_type TEXT,
                        permission TEXT,
                        description TEXT,
                        summary TEXT,
                        parent INTEGER,
                        value_type TEXT,
                        custom_value TEXT,
                        default_value TEXT,
                        range TEXT,
                        extends TEXT,
                        FOREIGN KEY (version_id) REFERENCES version(id),
                        FOREIGN KEY (parent) REFERENCES configures(id))''')
        conn.commit()
        conn.close()

    def update_table_structure(self, path) -> bool:
        '''更新表结构并完成数据迁移

        一次是修改旧结构为新结构，新旧表结构实现里有。
        一次是增加extends列
        '''
        try:
            if not os.path.exists(path):
                return False
            
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            cursor.execute('BEGIN TRANSACTION')

            # 检查是否是旧的表结构
            is_old = False
            has_extends = False
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='configures';")
            table_exists = cursor.fetchone()
            if table_exists:
                cursor.execute("PRAGMA table_info(configures);")
                columns = cursor.fetchall()
                
                for column in columns:
                    if column[1] == 'group_name':
                        is_old = True
                    if column[1] == 'extends':
                        has_extends = True
            else:
                conn.commit()
                conn.close()
                os.remove(path)
                self.logger.info(f'Data base table is invalid. Remove file {path}')
                return False
            
            if is_old:
                self.logger.info('Data base table is old structure. Update structure')
                # 如果是基础数据库，直接删掉重新生成
                if path  == self._ro_db:
                    os.remove(path)
                else:
                    cursor.execute('''CREATE TABLE IF NOT EXISTS new_configures
                            (id INTEGER PRIMARY KEY AUTOINCREMENT,
                            version_id INTEGER,
                            node_name TEXT,
                            node_type TEXT,
                            permission TEXT,
                            description TEXT,
                            summary TEXT,
                            parent INTEGER,
                            value_type TEXT,
                            custom_value TEXT,
                            default_value TEXT,
                            range TEXT,
                            extends TEXT,
                            FOREIGN KEY (version_id) REFERENCES version(id),
                            FOREIGN KEY (parent) REFERENCES configures(id))''')
                    if has_extends:
                        cursor.execute("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                                    "SELECT "
                                    "version_id,"
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN property "
                                    "WHEN group_name IS NOT NULL THEN group_name "
                                    "END, "
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN 'key' "
                                    "WHEN group_name IS NOT NULL THEN 'schema' "
                                    "END,"
                                    "permission,"
                                    "description,"
                                    "summary,"
                                    "parent,"
                                    "data_type,"
                                    "user_value,"
                                    "default_value,"
                                    "range,"
                                    "CASE "
                                    "WHEN 'extends' IN (SELECT name FROM pragma_table_info('configures')) THEN extends "
                                    "ELSE NULL "
                                    "END "
                                    "FROM configures;")
                    else:
                        cursor.execute("INSERT INTO new_configures (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range, extends) "
                                    "SELECT "
                                    "version_id,"
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN property "
                                    "WHEN group_name IS NOT NULL THEN group_name "
                                    "END, "
                                    "CASE "
                                    "WHEN property IS NOT NULL THEN 'key' "
                                    "WHEN group_name IS NOT NULL THEN 'schema' "
                                    "END,"
                                    "permission,"
                                    "description,"
                                    "summary,"
                                    "parent,"
                                    "data_type,"
                                    "user_value,"
                                    "default_value,"
                                    "range,"
                                    "NULL "
                                    "FROM configures;")
                    # 删除原表
                    cursor.execute("DROP TABLE configures;")

                    # 将新表重命名为原表的名称
                    cursor.execute("ALTER TABLE new_configures RENAME TO configures;")

                    # 查看app表的default_version是否存在单词拼写错误
                    cursor.execute("PRAGMA table_info(app);")
                    columns = cursor.fetchall()
                    column_names = [col[1] for col in columns]
                    if 'defualt_version' in column_names:
                        cursor.execute("ALTER TABLE app RENAME COLUMN defualt_version TO default_version;")

            conn.commit()
            conn.close()

            return True
        
        except Exception as e:
            err_msg = traceback.format_exc()
            self.logger.error(f'{err_msg}')
            return False
        
    def db2dict(self, path):
        '''读取数据库到字典中'''
        try:
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            cursor.execute('BEGIN TRANSACTION')

            cursor.execute('SELECT * FROM app')
            apps = cursor.fetchall()

            cursor.execute('SELECT * FROM version')
            versions = cursor.fetchall()

            cursor.execute('SELECT * FROM configures')
            configures = cursor.fetchall()

            conn.commit()
            conn.close()

            data = dictHelper()
            stack = []
            i = j = k = 0
            for i in range(len(apps)):
                app = apps[i][1]
                app_id = i + 1
                default_value = apps[i][2]
                data[app] = {}
                if(default_value):
                    data[app]['_default_version'] = default_value
                for j in range(j, len(versions)):
                    # app_id改变，退出版本循环
                    version_app_id = versions[j][1]
                    if version_app_id != app_id:
                        break
                    version = versions[j][2]
                    data[app][version] = {}
                    if versions[j][3]:
                        data[app][version]['compatible'] = versions[j][3]

                    stack.append((0, data[app][version]))
                    # 如果占用内存过大，可以改为在这里读取对应版本的所有配置
                    # 不再保存k值，每次循环从0开始
                    for k in range(k, len(configures)):
                        # version_id改变，退出循环
                        if configures[k][1] != j + 1:
                            stack.clear()
                            break
                        
                        while configures[k][7] != stack[-1][0]:
                            stack.pop()
                        
                        node_name = configures[k][2]
                        node_type = configures[k][3]
                        permission = configures[k][4]
                        description = configures[k][5]
                        summary = configures[k][6]
                        value_type = configures[k][8]
                        custom_value = configures[k][9]
                        default_value = configures[k][10]
                        value_range = configures[k][11]
                        extends = configures[k][12]

                        if node_type == 'schema':
                            tmp = stack[-1][1]
                            tmp[node_name] = {}
                            stack.append((configures[k][0], tmp[node_name]))
                            if permission is not None:
                                tmp[node_name]['_permission'] = permission
                            if description is not None:
                                tmp[node_name]['_description'] = description
                            if summary is not None:
                                tmp[node_name]['_summary'] = summary
                            if extends is not None:
                                tmp[node_name]['_extends'] = extends
                        elif node_type == 'key':
                            tmp = stack[-1][1]
                            tmp[node_name] = {}
                            if permission is not None:
                                tmp[node_name]['_permission'] = permission
                            if description is not None:
                                tmp[node_name]['_description'] = description
                            if summary is not None:
                                tmp[node_name]['_summary'] = summary
                            if value_type is not None:
                                tmp[node_name]['_type'] = value_type
                            if custom_value is not None:
                                tmp[node_name]['_value'] = custom_value
                            if default_value is not None:
                                tmp[node_name]['_default'] = default_value
                            if value_range is not None:
                                tmp[node_name]['_range'] = value_range
            return data
        
        except:
            err_msg = traceback.format_exc()
            self.logger.error(err_msg)
            return {}

    def dict2db(self, dict:dict, path:str):
        '''字典写入到数据库'''
        try:
            conn = sqlite3.connect(path)
            cursor = conn.cursor()

            conn.execute('BEGIN TRANSACTION')

            cursor.execute("SELECT name FROM sqlite_master WHERE type='table'")
            tables = cursor.fetchall()

            # 遍历所有表名，对每个表执行DELETE语句以删除所有行
            for table in tables:
                cursor.execute(f"DELETE FROM {table[0]}")

            app_id = 0
            version_id = 0
            apps = list(dict.keys())
            for i in range(0, len(apps)):
                app = apps[i]
                app_id = app_id + 1
                default_version = self._calculate_default_version(dict[app])
                cursor.execute("INSERT INTO app (app_name, default_version) VALUES (?,?)",(app, default_version))

                versions = list(dict[app].keys())
                for j in range(0, len(versions)):
                    version = versions[j]
                    if version == '_default_version':
                        continue

                    version_id = version_id + 1
                    cursor.execute("INSERT INTO version (app_id, version, compatible) VALUES (?, ?, ?)",
                                (app_id, version, dict[app][version].get('_compatible')))
                    
                    configures = dict[app][version]
                    self._write_group_in_db(configures, configures, cursor, version_id, 0)

            conn.commit()
            conn.close()

            return True
        
        except:
            err_msg = traceback.format_exc()
            self.logger.error(err_msg)
            conn.rollback()
            conn.commit()
            return False

    #递归写入组到configures表
    def _write_group_in_db(self, version_data:dict, data: dict, cursor: sqlite3.Cursor, version_id, parent_id):
        groups = []
        for key, value in data.items():
            if isinstance(value, dict):
                if(value.get('_default') is None):
                    groups.append(key)
                else:
                    self._write_key_in_db(version_data, key, value, cursor, version_id, parent_id, data.get('_permission'))

        for group in groups:
            node_name = group

            node_type = 'schema'

            permission = data[group].get('_permission')
            if permission is None:
                permission = 'public'

            description = data[group].get('_description')

            summary = data[group].get('_summary')
            
            extends = data[group].get('_extends')

            cursor.execute('''INSERT INTO configures 
                           (version_id, node_name, node_type, permission, description, summary, parent, extends) 
                           VALUES (?, ?, ?, ?, ?, ?, ?, ?)''',
                           (version_id, node_name, node_type, permission, description, summary, parent_id, extends))
            group_id = cursor.lastrowid
            self._write_group_in_db(version_data, data[group], cursor, version_id, group_id)

    # 写入键的数据到configures表内
    def _write_key_in_db(self, version_data:dict, key:str, value:dict, cursor: sqlite3.Cursor, version_id, parent_id, parent_permission):
        node_name = key

        node_type = 'key'

        permission = value.get('_permission')
        if permission is None:
            if parent_permission is None:
                permission = 'public'
            else:
                permission = parent_permission

        description = value.get('_description')

        summary = value.get('_summary')

        value_type = value.get('_type')

        custom_value = value.get('_value')

        default_value = value.get('_default')
        if isinstance(default_value, bool):
            default_value = 'true' if default_value else 'false'
        
        range = value.get('_range')
        if value_type == 'enum' and isinstance(range, str) and range.startswith('@'):
            enum_range = {}
            element_list = self._recursive_search(version_data, value['_range'][1:])
            for element in element_list:
                num = element['_value']
                if isinstance(num, str):
                    if num.startswith('0x'):
                        num = int(num, 16)
                    else:
                        num = int(num, 10)
                enum_range[element['_nick']] = num
            range = str(enum_range)
        elif value_type == 'bool':
            value_type = 'b'
        elif value_type == 'int':
            value_type = 'i'
        elif value_type == 'int64':
            value_type = 'x'
        elif value_type == 'uint':
            value_type = 'u'
        elif value_type == 'uint64':
            value_type = 't'
        elif value_type == 'double':
            value_type = 'd'
        elif value_type == 'string':
            value_type = 's'
        
        cursor.execute('''INSERT INTO configures 
                       (version_id, node_name, node_type, permission, description, summary, parent, value_type, custom_value, default_value, range) 
                       VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)''',
                       (version_id, node_name, node_type, permission, description, summary, parent_id, value_type, custom_value, default_value, range))

    def _calculate_default_version(self, data:dict):
        '''计算默认版本号'''
        versions = data.keys()
        if '_default_version' in versions:
            return data.get('_default_version')

        custom_versions = [CustomVersion(v) for v in versions]
        sorted_versions = sorted(custom_versions)
        
        return sorted_versions[-1].version_str
    
    def _recursive_search(self, dictionary, key):
        '''查询枚举类型数据的枚举定义'''
        if key in dictionary:
            return dictionary[key]
        else:
            for value in dictionary.values():
                if isinstance(value, dict):
                    result = self._recursive_search(value, key)
                    if result is not None:
                        return result
        return None
