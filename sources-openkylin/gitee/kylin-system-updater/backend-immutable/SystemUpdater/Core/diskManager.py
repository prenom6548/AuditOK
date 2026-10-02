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
import logging
import apt_pkg
from gettext import gettext as _
from SystemUpdater.Core.errors import *
from SystemUpdater.Core.enums import *

class diskManager():
    def __init__(self) -> None:
        self.mounted_list = None
        self.mounted_point_free_space = None
        self.init_finish = False
        self._start()

    def _start(self):
        try:
            self.mounted_list = self._get_mount_points()
            self.mounted_point_free_space = self._get_mounted_point_free_space()
            self.init_finish = True
        except Exception as e:
            logging.error(e)

    def _get_mount_points(self):
        # 获得系统的挂载点列表
        # get the mount points list from /proc/mounts
        mounted = []
        with open("/proc/mounts") as mounts:
            for line in mounts:
                try:
                    (what, where, fs, options, a, b) = line.split()
                except ValueError as e:
                    logging.debug("line '%s' in /proc/mounts not understood (%s)" % (line, e))
                    continue
        
                # 检查挂载点是否可写且实际存在
                if where == "/" and os.path.exists(where) and where not in mounted:
                    mounted.append(where)
                    break
            # make sure mounted is sorted by longest path
        mounted.sort(key=len, reverse=True)
        return mounted
    
    def _get_mounted_point_free_space(self):
        # 获取系统每个挂载点的剩余空间
        fs_free = {}
        for mount_point in self.mounted_list:
            statvfs = os.statvfs(mount_point)
            free = statvfs.f_frsize * statvfs.f_bavail # 剩余空间，单位 GB
            fs_free[mount_point] = free
        return fs_free
    
    def _get_mounted_point_by_dir(self, archivedir):
        """ return 'id' of a directory so that directories on the
            same filesystem get the same id (simply the mount_point)
        """
        for mount_point in self.mounted_list:
            if archivedir.startswith(mount_point):
                return mount_point
        return "/"

    # External interface
    def check_space_size_by_size(self, download_size, additional_size):
        '''
        download_size: 下载包的总大小
        additional_size: 除下载空间外还需要额外的空间大小
        通过输入的需求安装列表，计算挂载点大小
        安装下载的地址通过 archivedir = apt_pkg.config.find_dir("Dir::Cache::archives") 自动获取
        '''
        if self.init_finish is True:
            result = True
            install_req_size = {}
            notfree_mounted_pointed = []
            try:
                # 下载路径
                archivedir = apt_pkg.config.find_dir("Dir::Cache::archives")
                # V11上使用ostree，安装路径为/ostree/repo
                # 除安装空间外还预留一些安全空间缓冲区，设置为固定值
                for (dir, size) in [(archivedir, download_size),
                                            # 需要检测/ostree下是否足够安装，安装空间需求=下载大小+额外需要空间
                                            ("/ostree", download_size + additional_size),
                                            ("/", 500*1024*1024),     # small safety buffer /
                                            ]:
                    mounted_point = self._get_mounted_point_by_dir(dir)
                    if size < 0:
                        # 兼容出现负数的BUG
                        continue
                    if mounted_point in install_req_size:
                        install_req_size[mounted_point] += size
                    else:

                        install_req_size[mounted_point] = size
                    logging.info(f"dir {dir} needs {size} disk size")
                
                for mounted_point in install_req_size:
                    if self.mounted_point_free_space[mounted_point] > install_req_size[mounted_point]:
                        continue
                    else:
                        result = False
                        notfree_mounted_pointed.append(mounted_point)

                if result is False:
                    logging.error(f"Not enough free space:{notfree_mounted_pointed}")
                else:
                    logging.info("Check: Enough free space For system update...")
            except Exception as e:
                logging.error(e)
                result = False

        else:
            # 如果初始化失败，默认不要影响更新流程
            result = True
            logging.error("diskManager Initialization is not complete")
            return result

        return result
    
def check_free_space(download_size, addition_size=0):
    try:
        _diskManager = diskManager()
        result =_diskManager.check_space_size_by_size(download_size, addition_size)
        if result is False:
            raise UpdateBaseError(ERROR_NOT_DISK_SPACE)
    except Exception as e:
        logging.error(e)
        raise UpdateBaseError(ERROR_NOT_DISK_SPACE)
        return False
    return True