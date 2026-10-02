#!/usr/bin/python3
import os
import sys
import apt_pkg
import apt

def ReadOsReleaseFile():
    file = ''
    os_release_info={}
    try:
        if os.path.exists('/etc/os-release'):
            file = '/etc/os-release'
        elif os.path.exists('/usr/lib/os-release'):
            file = '/usr/lib/os-release'
        else:
            return os_release_info
        with open(file) as f:
            lines = f.readlines()
            for line in lines:
                ls = line.strip().split('=',1)
                os_release_info.update({ls[0]:ls[1].strip('\"').strip()})
    except Exception:
        pass
    return os_release_info

def get_package_version(packagename):
    cache = apt.Cache()
    if packagename in cache:
        pkg = cache[packagename]
        return pkg.installed.version
    else:
        return ''
    
def compare_package_version(v1,v2):
    return apt_pkg.version_compare(v1,v2)

if __name__ == "__main__":
    os_release_info = ReadOsReleaseFile()
    kylin_release_id = ''
    ret = 0
    try:
        if 'KYLIN_RELEASE_ID' in os_release_info:
            kylin_release_id = os_release_info['KYLIN_RELEASE_ID']
        ukui_control_center_version = get_package_version('ukui-control-center')
        res = compare_package_version(ukui_control_center_version,sys.argv[1])#'3.1.1+2022-0324'    
        if kylin_release_id in ["2203","2209"] or res >= 0: 
            ret = 1
        else:
            pass               
    except Exception as e:
        print(e)
    print(ret)
