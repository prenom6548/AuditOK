#!/usr/bin/python3

import os
import logging

class LocalUpgradeDataList:
    """
    Represent the (potentially partial) results of an unattended-upgrades
    run
    """
    def __init__(self):
        #可升级的组列表
        self._push_groups = []
        self._push_steps = []
        self._push_singles = []

        self._pkgs_data = {}

        #加版本号的升级包
        self.versoin_pkgs = {'single_upgrade':{}, 'groups_upgrade':{}}

        self._problem_resolver = {}
        self.steps_queue = []
        self.last_task = ''

        self.resolver_name = "total_pkgs"

        if "en_US" in os.environ["LANG"]:
            self.lang = "en_US"
        else:
            self.lang = "zh_CN"

    def refresh(self):
        self._push_groups = []
        self._push_steps = []
        self._push_singles = []

        self._pkgs_data = {}

        #加版本号的升级包
        self.versoin_pkgs = {'single_upgrade':{}, 'groups_upgrade':{}}

        self._problem_resolver = {}

        self.steps_queue = []
        self.last_task = ''

        if "en_US" in os.environ["LANG"]:
            self.lang = "en_US"
        else:
            self.lang = "zh_CN"

    #单包推送
    def add_singles(self,pkgs):
        self._push_singles = pkgs
    def get_singles(self):
        return self._push_singles
    
    #分步推送 添加分步数据
    def add_steps(self,groups,data):
        self._push_steps.append(groups)
        self._pkgs_data.update(data)

    def get_steps(self):
        return self._push_steps
    
    #组推送
    def add_groups(self,groups,data):
        self._push_groups.append(groups)
        self._pkgs_data.update(data)
    def get_groups(self):
        return self._push_groups

    #获取所以推送的升级
    def get_push(self):
        return self._push_groups + self._push_singles

    def is_steps(self,content):
        for cont in content:
            if cont in self._push_singles:
                return False
            
        if self._push_steps == []:
            return False
        else:
            return True

    def check_steps_reboot(self,content):
        return self._pkgs_data.get(content,{}).get('need_reboot',False)

    def get_pkgs_total(self,cache):
        pkgs_install = []
        pkgs_upgrade = []
        
        #单包的升级方式
        for pkg in self._push_singles:
            if cache[pkg].is_installed:
                pkgs_upgrade.append(pkg)
            else:
                pkgs_install.append(pkg) 

        #遍历升级组列表
        for group_name in self._push_groups:
            pkgs_install += self._pkgs_data.get(group_name,{}).get('install_list',[])
            pkgs_upgrade += self._pkgs_data.get(group_name,{}).get('upgrade_list',[]) 
        
        return pkgs_install,pkgs_upgrade  

    def get_steps_pkgs(self,step_name):
        pkgs_install = []
        pkgs_upgrade = []

        pkgs_install += self._pkgs_data.get(step_name,{}).get('install_list',[])
        pkgs_upgrade += self._pkgs_data.get(step_name,{}).get('upgrade_list',[]) 

        return pkgs_install,pkgs_upgrade 

    def get_push_pkgs(self,content):
        pkgs_install = []
        pkgs_upgrade = []

        if content == []:
            for group_name in self._push_groups:
                pkgs_install += self._pkgs_data.get(group_name,{}).get('pkgs_install',[])
                pkgs_upgrade += self._pkgs_data.get(group_name,{}).get('pkgs_upgrade',[]) 

            if self._push_singles != []:
                pkgs_upgrade += self._push_singles
        else:
            for cont in content:
                if cont in self._push_groups:
                    pkgs_install += self._pkgs_data.get(cont,{}).get('pkgs_install',[])
                    pkgs_upgrade += self._pkgs_data.get(cont,{}).get('pkgs_upgrade',[]) 

                if cont in self._push_singles:
                    pkgs_upgrade.append(cont)

        return pkgs_install,pkgs_upgrade

    #将要升级的内容
    def _make_upgrade_content(self,content):
        if content == []:
            return self._push_groups + self._push_singles
        else:
            return content

    def resolver_groups(self,pkgs_install,pkgs_upgrade,pkgs_remove):
        put_data = {self.resolver_name:{'install_pkgs':pkgs_install,'upgrade_pkgs':pkgs_upgrade,'reomve_pkgs':pkgs_remove}}
        self._problem_resolver.update(put_data)

    def resolver_steps(self,step_name,pkgs_install,pkgs_upgrade,pkgs_remove):
        put_data = {step_name:{'install_pkgs':pkgs_install,'upgrade_pkgs':pkgs_upgrade,'reomve_pkgs':pkgs_remove}}
        self._problem_resolver.update(put_data)

    def classify_content(self,content):
        if content == []:
            return self._push_singles.copy(),self._push_groups.copy()
        else:
            for cont in content:
                if cont in self._push_singles.copy():
                    return content,[]
                elif cont in self._push_groups.copy():
                    return [],content
                else:
                    return [],[]

    def get_resolver_upgrade(self):
        pkgs_install = []
        pkgs_upgrade = []
        pkgs_remove = []
        pkgs_install = self._problem_resolver.get(self.resolver_name,{}).get('install_pkgs',[])
        pkgs_upgrade = self._problem_resolver.get(self.resolver_name,{}).get('upgrade_pkgs',[])
        pkgs_remove = self._problem_resolver.get(self.resolver_name,{}).get('reomve_pkgs',[])

        return pkgs_install,pkgs_upgrade,pkgs_remove

    def get_progress(self,content):
        #为了 解决获取包列表和获取进度传输的数据不能协同的问题
        if content == self.resolver_name:
            for groups in self._push_groups:
                content = groups
                break
        begin = self._pkgs_data.get(content,{}).get('progress_begin',0)
        end = self._pkgs_data.get(content,{}).get('progress_end',0)

        return begin,end

    def pop(self,content):
        for cont in content:
            if cont in self._push_singles:
                self._push_singles.remove(cont)
            if cont in self._push_groups:
                self._push_groups.remove(cont)

    def get_resolver_steps(self,content):
        pkgs_install = []
        pkgs_upgrade = []
        pkgs_remove = []
        pkgs_install = self._problem_resolver.get(content,{}).get('install_pkgs',[])
        pkgs_upgrade = self._problem_resolver.get(content,{}).get('upgrade_pkgs',[])
        pkgs_remove = self._problem_resolver.get(content,{}).get('reomve_pkgs',[])

        return pkgs_install,pkgs_upgrade,pkgs_remove
    
    #升级完成后从升级列表删除
    def reset_push_content(self,content):
        for cont in content:
            if cont in self._push_singles:
                self._push_singles.remove(cont)
            if cont in self._push_groups:
                self._push_groups.remove(cont)

    def get_post_notify(self):
        notify = self._pkgs_data.get(self.last_task,{}).get("users_notify",{}).get("post_install_notify",None)
        if notify == None:
            return ''

        return notify.get(self.lang,'')

    def get_mid_notify(self,content):
        notify = self._pkgs_data.get(content,{}).get("users_notify",{}).get("mid_install_notify",None)
        if notify == None:
            return ''

        return notify.get(self.lang,'')

    def get_pre_script(self,content):
        script_list = []

        if content == self.resolver_name:
            for groups in self._push_groups:
                content = groups
                break

        script = self._pkgs_data.get(content,{}).get("script",None)
        if script == None:
            return script_list
        
        for scr in ["pre_update_status_check","pre_update_script"]:
            run_path = script.get(scr,[])

            if isinstance(run_path,list) == True:
                for sc in run_path:
                    if sc != '':
                        script_list.append(sc)
            elif isinstance(run_path,str) == True:
                if run_path != '':
                    script_list.append(run_path)
            else:
                pass

        return script_list

    def get_post_script(self,content):
        script_list = []
        if content == self.resolver_name:
            for groups in self._push_groups:
                content = groups
                break

        script = self._pkgs_data.get(content,{}).get("script",None)
        if script == None:
            return script_list
        
        for scr in ["post_update_status_check","post_update_script"]:
            run_path = script.get(scr,[])

            if isinstance(run_path,list) == True:
                for sc in run_path:
                    if sc != '':
                        script_list.append(sc)
            elif isinstance(run_path,str) == True:
                if run_path != '':
                    script_list.append(run_path)
            else:
                pass

        return script_list