/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ZhiMin Shao <shaozhimin@kylinos.cn>
 *
 */


#include "../libkypackages.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void install_call_back_install(int progress, int state, char *loginfo)
{
    printf("%s\n", loginfo);
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (geteuid() == 0) // root 用户，可以调用接口
    {
        if(argc < 2)
        {
            printf("使用方法: %s <deb文件路径>\n", argv[0]);
            return 0;
        }

        if (setenv("PACKAGE_KARE_INFO", "1", 1) != 0) 
        {
            return 0;
        }
        
        // 安装软件包
        ret = kdk_package_install_package(argv[1], KDK_PACKAGE_AUTO, install_call_back_install);
        
        // 清理资源
        kdk_package_unregister_install_package_handle();
    }
    else
    {
        printf("所请求的操作需要超级用户权限\n");
    }
    
    return ret;
}