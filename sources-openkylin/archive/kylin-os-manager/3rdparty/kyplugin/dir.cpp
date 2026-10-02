/*
 * Pluma - Plug-in Management Framework
 * Copyright (C) 2010-2012 Gil Costa (gsaurus@gmail.com)
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 *    If you use this software in a product, an acknowledgment
 *    in the product documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <dirent.h>

#include <cstdio>
#include <queue>

#include "dir.h"

namespace kyplugin
{
namespace dir
{

void listFiles(std::list<std::string> &list, const std::string &folder, const std::string &extension, bool recursive)
{
    DIR *dir;
    DIR *subDir;
    struct dirent *ent;

    dir = opendir(folder.c_str());
    if (dir == NULL) {
        fprintf(stderr, "Could not open \"%s\" directory.\n", folder.c_str());
        return;
    } else {
        closedir(dir);
    }

    std::queue<std::string> folders;
    folders.push(folder);

    while (!folders.empty()) {
        std::string currFolder = folders.front();
        folders.pop();

        dir = opendir(currFolder.c_str());
        if (dir == NULL) {
            continue;
        }

        while ((ent = readdir(dir)) != NULL) {
            std::string name(ent->d_name);

            /* 忽略 . 和 .. */
            if (name.compare(".") == 0 || name.compare("..") == 0) {
                continue;
            }

            std::string path = currFolder;
            path.append("/");
            path.append(name);

            /* 通过尝试打开该路径 , 判断是否是目录 */
            subDir = opendir(path.c_str());
            if (subDir != NULL) {
                closedir(subDir);
                if (recursive) {
                    folders.push(path);
                }
            } else {
                /* 是文件 */
                if (extension.empty()) {
                    list.push_back(path);
                } else {
                    /* 检查扩展是否一致 */
                    size_t lastDot = name.find_last_of('.');
                    std::string ext = name.substr(lastDot + 1);
                    if (ext.compare(extension) == 0) {
                        list.push_back(path);
                    }
                }
            }
        }
        closedir(dir);
    }
}

} // namespace dir
} // namespace kyplugin