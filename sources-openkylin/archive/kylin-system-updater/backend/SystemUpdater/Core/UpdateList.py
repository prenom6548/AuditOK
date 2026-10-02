# UpdateList.py

from gettext import gettext as _
import logging
import os
import json
import yaml
import shutil
from gi.repository import Gio
from .errors import *
from .enums import *
from SystemUpdater.Core.utils import get_config_patch

class UpdateList():
    OUTPUT_JSON_PATH = '/var/lib/kylin-system-updater/json/'
    IMPORTANT_LIST_PATH = '/var/lib/kylin-software-properties/template/important.list'

    def __init__(self,parent):
        self.parent = parent

        #所有的组升级安装列表
        self.update_meta = self.parent.main_meta

        if 'XDG_CURRENT_DESKTOP' in os.environ:
            self.current_desktop = os.environ.get('XDG_CURRENT_DESKTOP')
        else:
            self.current_desktop = ''

        if 'XDG_DATA_DIRS' in os.environ and os.environ['XDG_DATA_DIRS']:
            data_dirs = os.environ['XDG_DATA_DIRS']
        else:
            data_dirs = '/usr/local/share/:/usr/share/'
            
        self.application_dirs = [os.path.join(base, 'applications')
                                for base in data_dirs.split(':')]
            
    #清空上次输出的分组JSON文件
    def _empty_output_dir(self,path):
        #清空 升级列表
        if not os.path.exists(path):
            os.makedirs(path)
            logging.info('making the ConfigPath(%s) is complete...',path)
        else:
            shutil.rmtree(path)
            os.makedirs(path)
            logging.info('Emptying the ConfigPath(%s) is complete...',path)

    #读取推送列表，判断分组和单包推送，再进行源过滤
    def _make_important_list(self,cache,pkgs_upgrade,important_list = []):
        upgradeable_pkgs = []
        # tmp = []
        upgradeable_groups = []

        logging.info("The Server Push List: %a",important_list)

        for pkg_name in important_list:
            #检查是否在cache 没有在cache中属于组
            if pkg_name in cache:
                pkg_obj = cache[pkg_name]
                #在可升级的列表当中  此步骤为了排除已安装不需要升级的
                if pkg_obj.is_installed:
                    if pkg_name in pkgs_upgrade:
                        pkgs_upgrade.remove(pkg_name)
                        upgradeable_pkgs.append(pkg_obj.name)
                else:
                    upgradeable_pkgs.append(pkg_obj.name)
            else:
                upgradeable_groups.append(pkg_name)

        logging.info("Push Single Packages: %a, Push Groups:%a",upgradeable_pkgs,upgradeable_groups)
        return upgradeable_groups,upgradeable_pkgs

    def _make_pkg_info_json(self,cache,pkgs_list):
        total_download_size = 0
        total_installed_size = 0
        pkgs_info_json = {}

        for pkg_name in pkgs_list:
            pkg = cache[pkg_name]
            #当前版本
            cur_version = getattr(pkg.installed, "version", '')
            new_version = getattr(pkg.candidate, "version", '')

            #获取下载大小
            download_size = getattr(pkg.candidate, "size", 0)
            installed_size = getattr(pkg.candidate, "installed_size", 0)

            total_download_size = total_download_size + download_size
            total_installed_size = total_installed_size + installed_size

            pkgs_info_json.update({pkg_name:{"cur_version":cur_version,"new_version":new_version,\
                                            "download_size":str(download_size),"install_size":str(installed_size)}})

        pkgs_info_json.update({"total_download_size":str(total_download_size)})
        pkgs_info_json.update({"total_install_size":str(total_installed_size)})
        return pkgs_info_json

    #检查包是否在cache中 返回新得列表没 有安装的话才添加到列表
    def _check_pkg_in_cache(self,cache,pkgs_list):
        new_pkgs_list = []
        for pkg_name in pkgs_list:
            #检查是否在cache 以及 是否安装检查 
            if pkg_name in cache and not cache[pkg_name].is_installed:
                new_pkgs_list.append(pkg_name)
            else:
                pass
        return new_pkgs_list

    def _make_group_output_json(self,data,data_yaml,upgrade_pkgs_json,install_pkgs_json,output_path):
        groups_base_info = {}
        output_json = {}
        
        #FIXME: 确定输出文件的文件名 以及放置位置
        output_config_name = output_path + data['package'] + '.json'

        #4、添加一些基础信息
        groups_base_info.update({"package":data['package']})
        groups_base_info.update({"new_version":data['version']})
        groups_base_info.update({"name":data['name']})
        groups_base_info.update({"description":data['description']})
        groups_base_info.update({"icon":data['icon']})
        
        #添加读yaml文件
        groups_base_info.update({"changelog":data_yaml['changelog']})

        #5、添加升级的内容
        output_json.update(groups_base_info)
        output_json.update({"upgrade_list":upgrade_pkgs_json})
        output_json.update({"install_list":install_pkgs_json})
        # output_json.update({"hold_list":hold_pkgs_list})
        # output_json.update({"remove_list":remove_pkgs_list})

        #6 产生JSON文件
        with open(output_config_name, 'w', encoding='utf-8') as f:
            json.dump(output_json, f, ensure_ascii=False, indent=4)                   
        logging.info("Generate Jsonfile(%s) to complete... ",output_config_name)
    
    #输出白名单的配置
    def _make_autoupgrade_config(self,cache,output_path,config_path):
        pkgs_install,pkgs_upgrade = self.update_meta.get_pkgs_total(cache)

        output_config_name = output_path + 'auto-upgrade-list.json'
        output_json = {}
        install_info = {}
        for pkg in pkgs_install:
            pkg_cache = cache[pkg]
            pkgs_json = {}
            pkgs_json.update({"cur_version":getattr(pkg_cache.installed, "version", '')})

            pkgs_json.update({"new_version":getattr(pkg_cache.candidate, "version", '')})
            install_info.update({pkg:pkgs_json})

        upgrade_json = {}
        for pkg in pkgs_upgrade:
            pkg_cache = cache[pkg]
            pkgs_json = {}
            pkgs_json.update({"cur_version":getattr(pkg_cache.installed, "version", '')})

            pkgs_json.update({"new_version":getattr(pkg_cache.candidate, "version", '')})

            upgrade_json.update({pkg:pkgs_json})

        group_json = {}
        for ug in self.update_meta.get_groups():
            pkgs_json = {}
            with open(config_path + str(ug) + ".yaml", "r") as stream:
                try:
                    data_yaml = yaml.safe_load(stream)
                    pkgs_json.update({"cur_version":""})
                    pkgs_json.update({"new_version":data_yaml["version"]})
                    pkgs_json.update({"changelog":data_yaml["changelog"]})
                except yaml.YAMLError as exc:
                    logging.error(exc)
            group_json.update({ug:pkgs_json})

        single_json = {}
        for us in self.update_meta.get_singles():
            pkg_cache = cache[us]
            pkgs_json = {}
            pkgs_json.update({"cur_version":getattr(pkg_cache.installed, "version", '')})

            pkgs_json.update({"new_version":getattr(pkg_cache.candidate, "version", '')})
            pkgs_json.update({"changelog":""})
            single_json.update({us:pkgs_json})

        output_json.update({"upgrade_list":upgrade_json})
        output_json.update({"install_list":install_info})
        output_json.update({"group_json":group_json})
        output_json.update({"single_json":single_json})
            
        #产生JSON文件
        with open(output_config_name, 'w', encoding='utf-8') as f:
            json.dump(output_json, f, ensure_ascii=False, indent=4)                
        logging.info("Generate AutoUpgrade Configfile to Complete and Jsonfile(%s) to complete... ",output_config_name)

    def _split_package_id(self,package):
        """Return the name, the version number and the release of the
        specified package."""
        if "=" in package:
            name, version = package.split("=", 1)
            release = None
        elif "/" in package:
            name, release = package.split("/", 1)
            version = None
        else:
            name = package
            version = release = None
        return name, version, release

    def _make_downgrade(self,cache,downgrade_pkgs):
        output_downgrade = []
        adjust_pkgs = []
        for pkg_name, pkg_ver, pkg_rel in [self._split_package_id(pkg)
                                            for pkg in downgrade_pkgs]:
            try:
                pkg = cache[pkg_name]
            except KeyError:
                logging.warning("Package %s isn't available",pkg_name)
                continue
            if not pkg.is_installed:
                logging.warning("Package %s isn't installed",pkg_name)

            if pkg_ver:
                if pkg.installed and pkg.installed.version < pkg_ver:
                    logging.warning("The former version %s of %s is already installed",pkg.installed.version, pkg.name)
                    continue
                elif pkg.installed and pkg.installed.version == pkg_ver:
                    logging.warning("The version %s of %s is already installed",pkg.installed.version, pkg.name)
                    continue
                
                try:
                    pkg.candidate = pkg.versions[pkg_ver]
                except KeyError:
                    logging.warning("The version %s of %s isn't available",pkg_ver, pkg_name)
                    continue

                output_downgrade.append(pkg_name)
                adjust_pkgs.append(pkg_name+'='+pkg_ver)

        return output_downgrade,adjust_pkgs

    def _get_downgrade_list(self,cache,data):
        downgrade_pkgs = []

        try:
            downgrade_raw = data['force_install_list']
        except Exception as e:
            downgrade_raw = []

        for pkg_name, pkg_ver, pkg_rel in [self._split_package_id(pkg)
                                            for pkg in downgrade_raw]:
            
            if pkg_name in cache:
                downgrade_pkgs.append(pkg_name)
            else:
                logging.warning("Package %s isn't available",pkg_name)
                continue

        return downgrade_raw,downgrade_pkgs

    def _make_groups_pkgs(self,cache,data,pkgs_upgrade = []):

        upgrade_pkgs_list =  data['upgrade_list']
        new_install_list = self._check_pkg_in_cache(cache,data['install_list'])

        #进行交集 升级列表
        new_upgrade_list = list(set(pkgs_upgrade) & set(upgrade_pkgs_list))

        #在总升级列表中移除这些包
        for pkg in new_upgrade_list:
            pkgs_upgrade.remove(pkg)

        #单包的优先级最高 从组中剔除此包
        for pkg in self.update_meta.get_singles():
            if pkg in new_install_list:
                new_install_list.remove(pkg)

        return new_install_list,new_upgrade_list

    def _make_groups_upgrade(self,cache,group_list,is_openkylin,pkgs_install,pkgs_upgrade,output_path,config_path): 
        upgrade_list = []
        install_list = []

        if os.path.isdir(config_path) == False:
            logging.warning("configPath(%s) is not exists...",config_path)
            return

        files = os.listdir(config_path) #获得文件夹中所有文件的名称列表

        for ifile in files:
            if ifile.endswith('.json'):
                #读取组JSON文件
                with open(config_path+ifile,'r') as f:
                    try:
                        data = json.load(f)
                    except Exception as exc:
                        logging.error(exc)
                        raise UpdateBaseError(ERROR_LOAD_CONFIG_FAILED)

                group_name = data['package']
                #读取组的yaml 文件的changelog的信息
                with open(config_path + group_name + ".yaml", "r") as stream:
                    try:
                        data_yaml = yaml.safe_load(stream)
                    except Exception as exc:
                        logging.error(exc)
                        raise UpdateBaseError(ERROR_LOAD_CONFIG_FAILED)

                #过滤没有推送的配置文件
                if not group_name in group_list:
                    continue

                if is_openkylin == True:
                    install_list,upgrade_list = pkgs_install,pkgs_upgrade
                else:
                    install_list,upgrade_list = self._make_groups_pkgs(cache,data,pkgs_upgrade)

                #判断当前是否可升级或者新装的包
                if len(install_list) == 0 and len(upgrade_list) == 0:
                    continue

                #3、生成升级的包列表JSON
                upgrade_pkgs_json = self._make_pkg_info_json(cache,upgrade_list)
                #2、生成安装的软件列表
                install_pkgs_json = self._make_pkg_info_json(cache,install_list)
                #输出JSON配置文件
                self._make_group_output_json(data,data_yaml,upgrade_pkgs_json,install_pkgs_json,output_path)

                #保存分组版本号,好像没有
                self.update_meta.versoin_pkgs['groups_upgrade'].update({group_name:''})
                
                update_script = data.setdefault("script",{})
                
                steps_begin = 50
                step_update = data.setdefault("step_update",{})
                if step_update != {}:
                    for step_name in step_update:
                        step_data = step_update.get(step_name,{})
                        if step_data == {}:
                            continue
                        step_install = list(set(step_data.setdefault("install_list",[])) & set(install_list))
                        step_upgrade = list(set(step_data.setdefault("upgrade_list",[])) & set(upgrade_list))
                        need_reboot = step_data.setdefault("need_reboot",False)
                        if step_install == [] and  step_upgrade == []:
                            continue
                        step_data["install_list"] = step_install
                        step_data["upgrade_list"] = step_upgrade

                        #进度算法
                        value = int((len(step_install) + len(step_upgrade))/(len(install_list) + len(upgrade_list)) * 50)                        
                        if need_reboot == True:
                            step_data.update({"progress_begin":steps_begin})
                            step_data.update({"progress_end":100})
                        else:
                            progress_end = steps_begin + value
                            step_data.update({"progress_begin":steps_begin})
                            step_data.update({"progress_end":progress_end})
                            steps_begin = progress_end
                        
                        logging.info("Steps(%s) upgrade:%d install:%d",step_name,len(step_upgrade),len(step_install))
                        self.update_meta.add_steps(step_name,{step_name:step_data})

                        #需要重启的话 后面的阶段将不再考虑
                        if need_reboot == True:
                            break

                put_data = {group_name:{"progress_begin":steps_begin,"progress_end":100,"pkgs_upgrade":upgrade_list,"pkgs_install":install_list,"script":update_script}}

                self.update_meta.add_groups(group_name,put_data)
                logging.info("Group(%s) upgrade:%d install:%d",group_name,len(upgrade_list),len(install_list))
            else:
                pass

    def _rate_application_for_package(self, application, pkg):
        score = 0
        desktop_file = os.path.basename(application.get_filename())
        application_id = os.path.splitext(desktop_file)[0]

        if application.should_show():
            score += 1

            if application_id == pkg.name:
                score += 5

        return score

    def _file_is_application(self, file_path):
        # WARNING: This is called often if there's a lot of updates. A poor
        # performing call here has a huge impact on the overall performance!
        if not file_path.endswith(".desktop"):
            # First the obvious case: If the path doesn't end in a .desktop
            # extension, this isn't a desktop file.
            return False

        file_path = os.path.abspath(file_path)
        for app_dir in self.application_dirs:
            if file_path.startswith(app_dir):
                return True
        return False

    def get_application_for_package(self, pkg):
        desktop_files = []
        rated_applications = []

        for installed_file in pkg.installed_files:
            if self._file_is_application(installed_file):
                desktop_files.append(installed_file)

        for desktop_file in desktop_files:
            try:
                application = Gio.DesktopAppInfo.new_from_filename(
                    desktop_file)
                application.set_desktop_env(self.current_desktop)
            except Exception as e:
                logging.warning("Error loading .desktop file %s: %s" %
                                (desktop_file, e))
                continue
            score = self._rate_application_for_package(application, pkg)
            if score > 0:
                rated_applications.append((score, application))

        rated_applications.sort(key=lambda app: app[0], reverse=True)
        if len(rated_applications) > 0:
            return rated_applications[0][1]
        else:
            return None

    def _make_single_upgrade(self,cache,pkg_list,output_path):
        for pkg in pkg_list:
            zh_name = ''
            base_info = {}
            output_json = {}
            output_config_name = output_path + pkg + '.json'
            
            pkg_cache = cache[pkg]

            #获取包的软件名称，从主题中读取，只有可升级，是软件的包才可以读取
            if pkg_cache.is_installed:
                app = self.get_application_for_package(pkg_cache)
                if app is not None:
                    zh_name = app.get_display_name()
            
            pkgs_json = self._make_pkg_info_json(cache,[pkg])
            en_name = getattr(pkg_cache.candidate, "summary", '')
            description_str = getattr(pkg_cache.candidate, "description", '')

            #4、添加一些基础信息
            base_info.update({"package":pkg})

            base_info.update({"cur_version":getattr(pkg_cache.installed, "version", '')})
            base_info.update({"new_version":getattr(pkg_cache.candidate, "version", '')})
            base_info.update({"name":{"zh_CN":zh_name,"en_US":en_name}})
            base_info.update({"description":{"zh_CN":description_str,"en_US":description_str}})
            base_info.update({"icon":''})

            #5、添加升级的内容
            output_json.update(base_info)
            if pkg_cache.is_installed:
                output_json.update({"upgrade_list":pkgs_json})
                output_json.update({"install_list":{}})
            else:
                output_json.update({"upgrade_list":{}})
                output_json.update({"install_list":pkgs_json})
            
            #产生JSON文件
            with open(output_config_name, 'w', encoding='utf-8') as f:
                json.dump(output_json, f, ensure_ascii=False, indent=4)                
            logging.info("Generate Jsonfile(%s) to complete... ",output_config_name)

            #6、保存单包版本号
            self.update_meta.versoin_pkgs['single_upgrade'].update({pkg_cache.name:getattr(pkg_cache.installed, "version", '')})

    def _make_distupgrade(self,cache):
        pkgs_upgrade = []
        pkgs_install = []
        if cache.get_changes():
            cache.clear()
        cache._depcache.upgrade(True)

        #查找所有可升级的包
        for pkg in cache:
            try:
                if pkg.marked_install:
                    pkgs_install.append(pkg.name)
                elif pkg.marked_upgrade:
                    pkgs_upgrade.append(pkg.name)
            except KeyError:
                pass
        return pkgs_install,pkgs_upgrade

    def update_kylin(self,cache,important_data,is_openkylin = False):
        system_install = []
        system_upgrade = []

        #查找所有可升级的包
        if is_openkylin == True:
            system_install,system_upgrade = self._make_distupgrade(cache)
        else:
            for pkg in cache:
                if pkg.is_upgradable and pkg.is_installed:
                    system_upgrade.append(pkg.name)

        logging.info("System all upgradeable packages:upgrade:%d install:%d ",len(system_upgrade),len(system_install))

        group_list,single_pkgs = self._make_important_list(cache,system_upgrade,important_data)

        #清空输出的目录
        self._empty_output_dir(self.OUTPUT_JSON_PATH)

        if not group_list and not single_pkgs:
            return

        self.update_meta.add_singles(single_pkgs)

        #产生单包的JSON
        self._make_single_upgrade(cache,single_pkgs,self.OUTPUT_JSON_PATH)

        #分组的包的JSON
        self._make_groups_upgrade(cache,group_list,is_openkylin,\
                        system_install,system_upgrade,self.OUTPUT_JSON_PATH,get_config_patch())


        self._make_autoupgrade_config(cache,self.OUTPUT_JSON_PATH,get_config_patch())
        return 