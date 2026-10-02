#!/usr/bin/python3
import apt
import apt_pkg
import fnmatch
import logging
import logging.handlers
import re
import os
import sys
import string
import subprocess
import json
try:
    from typing import List
    from typing import Union
except ImportError:
    pass

from gettext import gettext as _

SYSTEM_UPDATER_CORE_LIB_PATH="/usr/share/kylin-system-updater/"
sys.path.append(SYSTEM_UPDATER_CORE_LIB_PATH)
from SystemUpdater.Core.utils import get_config_patch

ImportantListPath="/var/lib/kylin-software-properties/template/important.list"
SOURCESLIST = "/etc/apt/sources.list"

# no py3 lsb_release in debian :/
DISTRO_CODENAME = subprocess.check_output(
    ["lsb_release", "-c", "-s"], universal_newlines=True).strip()  # type: str
DISTRO_DESC = subprocess.check_output(
    ["lsb_release", "-d", "-s"], universal_newlines=True).strip()  # type: str
DISTRO_ID = subprocess.check_output(
    ["lsb_release", "-i", "-s"], universal_newlines=True).strip()  # type: str

ARCHITECTUREMAP = ['arm64','amd64','armhf','i386','loongarch64','mips64el','sw64']

RELEASEOFFSET = 1
ORIGINOFFSET  = 2
HTTPTYPE = "HTTP"
FTPTYPE  = "FTP"

class UpdateListFilterCache(apt.Cache):

    def __init__(self, window_main):
        self.window_main = window_main
        # whitelist
        self.upgradeList = []
        # 必须升级的包
        self.installList = []
        
        self.config_path = get_config_patch()
        
        # 获取源属性
        self.origin_property = OriginProperty()
        self.origin_property.get_allowed_sources()
        self.origin_property.get_allowed_origin()

        self.allowed_origins = get_allowed_origins(self.origin_property.allow_origin)
        
        self.allowed_origins = deleteDuplicatedElementFromList(self.allowed_origins)
        logging.info(_("Allowed origins: %s"),
                    self.allowed_origins)

        # self.blacklist = apt_pkg.config.value_list(
        #     "Kylin-system-updater::Package-Blacklist")
        # self.blacklist = deleteDuplicatedElementFromList(self.blacklist)

        # self.whitelist = apt_pkg.config.value_list(
        #     "Kylin-system-updater::Package-Whitelist")
        # self.whitelist = deleteDuplicatedElementFromList(self.whitelist)

        # self.strict_whitelist = apt_pkg.config.find_b(
        #     "Kylin-system-updater::Package-Whitelist-Strict", False)

    def checkInCache(self):
        logging.info("start Check in cache")
        tmplist = []
        cache = apt.Cache()
        for i in self.upgradeList:
            try:
                cache[i]
                tmplist.append(i)
            except Exception as e:
                pass
        self.upgradeList = tmplist

    def initLocalPackagesList(self):
        jsonfiles = []
        tmplist = []

        # 获取importantlist 本次更新推送
        with open(ImportantListPath, 'r') as f:
            text = f.read()
        importantList = text.split()
        logging.info("importantList: %s",importantList)
        f.close()

        if not importantList:
            logging.error("importantList is empty")
            exit(-1)

        # 获取/usr/share/kylin-update-desktop-config/data/下所有json文件
        for root,dirs,files in os.walk(self.config_path):
            pass
        for i in files:
            if ".json" in i:
                jsonfiles.append(i.split('.')[0])

        # 找到importantlist中对应的json文件
        for i in importantList:
            if i not in jsonfiles:
                # 说明这个是单独的包，不在分组中
                # 加入更新列表
                if i not in self.upgradeList:
                            self.upgradeList.append(i)
            else:
                # 在分组中
                # 获取每个对应json文件中的upgrade_list
                if i in jsonfiles:
                    filepath = os.path.join(self.config_path, i)
                    filepath = filepath+".json"
                    with open(filepath, 'r') as f:
                        pkgdict = f.read()
                    jsonfile = json.loads(pkgdict)
                    tmplist = jsonfile['install_list']
                    for j in tmplist:
                        if j not in self.upgradeList:
                            self.upgradeList.append(j)
                    f.close()

    # 更改：传入包列表，经过源过滤，返回的pkg中进行版本调整
    def check_in_allowed_origin(self, pkg_lists, _is_adjust): 
        new_upgrade_pkgs = []
        adjust_candidate_pkgs = []
        for pkg in pkg_lists:
            try:
                new_ver = ver_in_allowed_origin(pkg, self.allowed_origins)
                if _is_adjust and len(new_ver) == 0:
                    logging.warning("< %s > did not find a suitable version..." % pkg.name)
                    continue
                if len(new_ver) == 0:
                    continue
                if not pkg.installed:   # 判断安装列表                        
                    if pkg.candidate == new_ver[0] and pkg not in new_upgrade_pkgs:
                        new_upgrade_pkgs.append(pkg)
                    elif new_ver[0] != pkg.candidate and pkg not in new_upgrade_pkgs:
                        logging.info("adjusting candidate version: %s" % new_ver[0])
                        if _is_adjust == True:
                            pkg.candidate = new_ver[0]
                        adjust_candidate_pkgs.append(pkg.name+"="+pkg.candidate.version)
                        new_upgrade_pkgs.append(pkg)
                else:                   # 判断升级列表
                    for nv in new_ver:
                        if nv > pkg.installed and nv != pkg.candidate:
                            logging.info("adjusting candidate version: %s" % nv)
                            if _is_adjust == True:
                                pkg.candidate = nv
                            adjust_candidate_pkgs.append(pkg.name+"="+pkg.candidate.version)
                            break
                        elif nv > pkg.installed and nv == pkg.candidate:
                            new_upgrade_pkgs.append(pkg)
                            break
                        elif _is_adjust == True:
                            logging.warning("< %s > did not find a suitable version..." % pkg.name)
            except NoAllowedOriginError:
                logging.error("Cannot found allowed version: %s", pkg.name)
                continue

        return (new_upgrade_pkgs, adjust_candidate_pkgs)

    def is_pkgname_in_blacklist(self, pkgs):
        blacklist_filter_pkgs = []
        for pkg in pkgs:
            if pkg.name in self.blacklist:
                pass
            else :
                blacklist_filter_pkgs.append(pkg)

        return blacklist_filter_pkgs

    def is_pkgname_in_whitelist(self, pkgs):
        whitelist_filter_upgrade_pkgs = []
        for pkg in pkgs:
            if pkg.name in self.upgradeList:
                whitelist_filter_upgrade_pkgs.append(pkg)
            else :
                pkg.mark_keep()
        return whitelist_filter_upgrade_pkgs

class OriginProperty():
    
    def __init__(self):
        # 包含了本地所有源 http & ftp
        self.local_sourcelist = {"http":[],"ftp":[]}
        # 经过解析后的本地源，获取所有的分发属性
        self.local_origin     = {"http":[],"ftp":[]}
        # 允许的源列表
        self.allow_sources = []
        # 允许的源+属性
        self.allow_origin = {"http":[],"ftp":[]}
        # 加载本地所有源
        self.init_local_origin()
        # 进行属性解析
        self.analytic_properties(self.local_sourcelist)

    def init_local_origin(self):
        http_origin = {}
        ftp_orgin   = {}
        #apt policy
        sh_retval = os.popen("apt-cache policy").read().split("\n")
        # policy = [ rv for rv in sh_retval if "http" in rv or "ftp" in rv or "release" in rv or "origin" in rv]
        for rv in sh_retval:
            if "http" in rv:
                http_origin['sources'] = rv
                http_origin['release'] = sh_retval[sh_retval.index(rv) + RELEASEOFFSET]
                http_origin['origin']  = sh_retval[sh_retval.index(rv) +  ORIGINOFFSET]
                self.local_sourcelist['http'].append(http_origin.copy())
            elif "ftp" in rv:
                ftp_orgin['sources'] = rv
                ftp_orgin['release'] = sh_retval[sh_retval.index(rv) + RELEASEOFFSET]
                ftp_orgin['origin']  = sh_retval[sh_retval.index(rv) +  ORIGINOFFSET]
                self.local_sourcelist['ftp'].append(ftp_orgin.copy())
    
    def merge_origin(self, source_type, source_origin):
        is_append = True
        if source_type == HTTPTYPE:
            if self.local_origin['http']:
                for lo in self.local_origin['http']:
                    if lo['origin_source'] == source_origin['origin_source'] and lo['dist'] == source_origin['dist']:
                        lo['component'] = list(set(lo['component']).union(set(source_origin['component'])))
                        is_append = False
                if is_append:
                    self.local_origin['http'].append(source_origin.copy())
            else:
                self.local_origin['http'].append(source_origin.copy())
        elif source_type == FTPTYPE:
            if self.local_origin['ftp']:
                for lo in self.local_origin['ftp']:
                    if lo['origin_source'] == source_origin['origin_source'] and lo['dist'] == source_origin['dist']:
                        lo['component'] = list(set(lo['component']).union(set(source_origin['component'])))
                        is_append = False
                if is_append:
                    self.local_origin['ftp'].append(source_origin.copy())
            else:
                self.local_origin['ftp'].append(source_origin.copy())

    def analytic_properties(self, local_sourcelist):
        http_origin = {"component":[],"release":{}}
        ftp_orgin   = {"component":[],"release":{}}
        dist_list = []
        # 经过解析后的本地源，获取所有的分发属性
        for ls in local_sourcelist['http']:
            for item in filter(not_empty, ls['sources'].split(' ')):
                if item.isdigit():
                    http_origin['policy_priority'] = item
                elif "http" in item:
                    http_origin['origin_source']   = item
                elif "/" in item:
                    dist_list = item.split("/")
                    dist_list.pop()
                    http_origin['dist'] = "/".join(dist_list)
                    http_origin['component'].append(item.split("/")[1])
                elif item not in ARCHITECTUREMAP and item != "Packages":
                    http_origin['component'].append(item)
            release_list = ls['release'].split(',')
            release_list = [ rl.strip() for rl in release_list ]
            if "release" in release_list[0]:
                release_list[0] = release_list[0].lstrip("release").strip()
            for rl in release_list:
                if "=" in rl:
                    self.generate_dict(http_origin['release'], rl)
            for item in filter(not_empty, ls['origin'].split(' ')):
                if "origin" not in ls['origin']:
                    break
                elif "origin" != item:
                    http_origin['origin'] = item
            self.merge_origin(HTTPTYPE, http_origin)
            http_origin = {"component":[],"release":{}}

        for ls in local_sourcelist['ftp']:
            for item in filter(not_empty, ls['sources'].split(' ')):
                if item.isdigit():
                    ftp_orgin['policy_priority'] = item
                elif "ftp" in item:
                    ftp_orgin['origin_source']   = item
                elif "/" in item:
                    ftp_orgin['dist'] = item.split("/")[0]
                    ftp_orgin['component'].append(item.split("/")[1])
                elif item not in ARCHITECTUREMAP and item != "Packages":
                    ftp_orgin['component'].append(item)
            release_list = ls['release'].split(',')
            if "release " in release_list[0]:
                release_list[0] = release_list[0].lstrip("release ")
            for rl in release_list:
                if "=" in rl:
                    self.generate_dict(ftp_orgin['release'], rl)
            for item in filter(not_empty, ls['origin'].split(' ')):
                if "origin" not in ls['origin']:
                    break
                elif "origin" != item:
                    ftp_orgin['origin'] = item
            self.merge_origin(FTPTYPE, ftp_orgin)
            ftp_orgin   = {"component":[],"release":{}}

    def generate_dict(self, dict, item):
        item = item.strip()
        if item == "":
            logging.warning("empty match string matches nothing")
            return False
        (what, value) = [ s for s in item.split("=")]
        if what in ('o', 'origin'):
            dict['origin'] = value
        elif what in ("l", "label"):
            dict['label'] = value
        elif what in ("a", "suite", "archive"):
            dict['archive'] = value
        elif what in ("c", "component"):
            dict['component'] = value
        elif what in ("site",):
            dict['site'] = value
        elif what in ("n", "codename",):
            dict['codename'] = value
        else:
            dict[what] = value
            # raise UnknownMatcherError(
            #     "Unknown whitelist entry for matcher %s (value %s)" % (
            #         what, value))
        
    def get_allowed_sources(self):
        # 源地址，在本地源列表中查找. 源服务器下发source.list为允许的源, 本模块屏蔽了sources.list.d下的源
        # 获取允许的源
        try:
            old_sources_list = apt_pkg.config.find("Dir::Etc::sourcelist")
            old_sources_list_d = apt_pkg.config.find("Dir::Etc::sourceparts")
            old_cleanup = apt_pkg.config.find("APT::List-Cleanup")
            apt_pkg.config.set("Dir::Etc::sourcelist",
                                os.path.abspath(SOURCESLIST))
            apt_pkg.config.set("Dir::Etc::sourceparts", "xxx")
            apt_pkg.config.set("APT::List-Cleanup", "0")
            slist = apt_pkg.SourceList()
            slist.read_main_list()
            self.allow_sources = slist.list
        except Exception as e:
            logging.error(str(e))
        finally:
            apt_pkg.config.set("Dir::Etc::sourcelist",
                                       old_sources_list)
            apt_pkg.config.set("Dir::Etc::sourceparts",
                                       old_sources_list_d)
            apt_pkg.config.set("APT::List-Cleanup",
                                       old_cleanup)

    def get_allowed_origin(self):
        # 获取允许的源
        # 生成源与属性
        self.local_origin
        self.allow_sources
        self.allow_origin
        try:
            for item in self.allow_sources:
                for lo in self.local_origin['http']:
                    if item.uri.strip('/') == lo['origin_source'].strip('/') and item.dist == lo['dist']:
                        self.allow_origin['http'].append(lo)
                for lo in self.local_origin['ftp']:
                    if item.uri.strip('/') == lo['origin_source'].strip('/') and item.dist == lo['dist']:
                        self.allow_origin['ftp'].append(lo)
        except Exception as e:
            logging.error(str(e))

class UnattendUpgradeFilter():
    def __init__(self) -> None:
        pass

    def GetAllowOrigins(self):
        # 获取源属性
        self.origin_property = OriginProperty()
        self.origin_property.get_allowed_sources()
        self.origin_property.get_allowed_origin()

        self.allowed_origins = get_allowed_origins(self.origin_property.allow_origin)
        
        self.allowed_origins = deleteDuplicatedElementFromList(self.allowed_origins)
        logging.info(_("Allowed origins: %s"),
                    self.allowed_origins)
        return self.allowed_origins
        

def ver_in_allowed_origin(pkg, allow_origin):
    # type: (apt.Package, List[str]) -> apt.package.Version
    allown_versions = []
    versions = _get_priority_order(pkg)
    # 获取每个优先级别中 允许源的最高版本
    allown_versions = _get_allowed_list(versions, allow_origin)
    
    return allown_versions

def _get_priority_order(pkg):
    versions = []
    for ver in pkg.versions:
        if versions:
            for v in versions:
                if v.policy_priority >= ver.policy_priority and v == versions[-1]: 
                    break
                elif v.policy_priority >= ver.policy_priority and v != versions[-1]: 
                    continue
                else:
                    index = versions.index(v)
                    versions.insert(index,ver)
                    break
            if v == versions[-1] and versions[-1].policy_priority >= ver.policy_priority:
                versions.append(ver)
        else:
            versions.append(ver)
    return versions

def _get_allowed_list(versions, allow_origin):
    current_priority = -100
    allown_versions = []
    for ver in versions:
        if current_priority != ver.policy_priority:
            if is_in_allowed_origin(ver, allow_origin):
                allown_versions.append(ver)
                current_priority = ver.policy_priority
        else:
            continue
    return allown_versions

def get_allowed_origins(allow_origin):
    """ return a list of allowed origins
    """
    allowed_origins = []
    origin  = ''
    archive = ''
    uri = ''
    label = ''
    for ao in (allow_origin['http']+allow_origin['ftp']):
        if 'origin' in ao['release']:
            origin = 'o='+ao['release']['origin']
        else:
            origin = 'o='
        if 'archive' in ao['release']:
            archive = 'a='+ao['release']['archive']
        else:
            archive = 'a='
        if 'label' in ao['release']:
            label = 'l='+ao['release']['label']
        else:
            label = 'l='
        if 'origin_source' in ao:
            uri = 'uri='+ao['origin_source']
        else:
            uri = 'uri='
        allowed_origins.append(origin+","+archive+","+label+","+uri)
    return allowed_origins

def get_allowed_origins_legacy():
    # type: () -> List[str]
    """ legacy support for old Allowed-Origins var """
    allowed_origins = []  # type: List[str]
    key = "Kylin-system-updater::Allowed-Origins"
    try:
        for s in apt_pkg.config.value_list(key):
            # if there is a ":" use that as seperator, else use spaces
            if re.findall(r'(?<!\\):', s):
                (distro_id, distro_codename) = re.split(r'(?<!\\):', s)
            else:
                (distro_id, distro_codename) = s.split()
            # unescape "\:" back to ":"
            distro_id = re.sub(r'\\:', ':', distro_id)
            # escape "," (see LP: #824856) - can this be simpler?
            distro_id = re.sub(r'([^\\]),', r'\1\\,', distro_id)
            distro_codename = re.sub(r'([^\\]),', r'\1\\,', distro_codename)
            # convert to new format
            allowed_origins.append("o=%s,a=%s" % (substitute(distro_id),
                                   substitute(distro_codename)))
    except ValueError:
        logging.error(_("Unable to parse %s." % key))
        raise
    return allowed_origins

def substitute(line):
    # type: (str) -> str
    """ substitude known mappings and return a new string

    Currently supported ${distro-release}
    """
    mapping = {"distro_codename": get_distro_codename(),
               "distro_id": get_distro_id()}
    return string.Template(line).substitute(mapping)


def get_distro_codename():
    # type: () -> str
    return DISTRO_CODENAME


def get_distro_id():
    # type: () -> str
    return DISTRO_ID

def is_in_allowed_origin(ver, allowed_origins):
    # type: (apt.package.Version, List[str]) -> bool
    if not ver:
        return False
    for origin in ver.origins:
        if is_allowed_origin(origin, allowed_origins):
            return True
    return False

def is_allowed_origin(origin, allowed_origins):
    # type: (Union[apt.package.Origin, apt_pkg.PackageFile], List[str]) -> bool
    for allowed in allowed_origins:
        if match_whitelist_string(allowed, origin):
            return True
    return False

def match_whitelist_string(whitelist, origin):
    # type: (str, Union[apt.package.Origin, apt_pkg.PackageFile]) -> bool
    """
    take a whitelist string in the form "origin=Debian,label=Debian-Security"
    and match against the given python-apt origin. A empty whitelist string
    never matches anything.
    """
    whitelist = whitelist.strip()
    if whitelist == "":
        logging.warning("empty match string matches nothing")
        return False
    res = True
    # make "\," the html quote equivalent
    whitelist = whitelist.replace("\\,", "%2C")
    for token in whitelist.split(","):
        # strip and unquote the "," back
        (what, value) = [s.strip().replace("%2C", ",")
                         for s in token.split("=")]
        # logging.debug("matching %s=%s against %s" % (
        #              what, value, origin))
        # support substitution here as well
        value = substitute(value)
        # first char is apt-cache policy output, send is the name
        # in the Release file
        if what in ("o", "origin"):
            match = fnmatch.fnmatch(origin.origin, value)
        elif what in ("l", "label"):
            match = fnmatch.fnmatch(origin.label, value)
        elif what in ("a", "suite", "archive"):
            match = fnmatch.fnmatch(origin.archive, value)
        elif what in ("c", "component"):
            match = fnmatch.fnmatch(origin.component, value)
        elif what in ("site",):
            match = fnmatch.fnmatch(origin.site, value)
        elif what in ("n", "codename",):
            match = fnmatch.fnmatch(origin.codename, value)
        elif what in ("uri",):
            match = True
        else:
            raise UnknownMatcherError(
                "Unknown whitelist entry for matcher %s (token %s)" % (
                    what, token))
        # update res
        res = res and match
        # logging.debug("matching %s=%s against %s" % (
        #              what, value, origin))
    return res

def deleteDuplicatedElementFromList(list):
    resultList = []
    for item in list:
        if not item in resultList:
            resultList.append(item)
    return resultList

def not_empty(s):
    return s and s.strip()

class UnknownMatcherError(ValueError):
    pass

class NoAllowedOriginError(ValueError):
    pass
