/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "fileutil.h"

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "utils/logger.h"

FileUtil::FileUtil() {}

bool FileUtil::isFileEmpty(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOGE("EngineConfiguration isFileEmpty: Failed to open file {}.", filePath);
        return true;
    }

    // 获取文件大小
    std::streamsize fileSize = file.tellg();
    file.close();

    return fileSize == 0;
}

bool FileUtil::createDirectory(const std::string &fileDir) {
    char *p = NULL;
    char *tmpPath = strdup(fileDir.c_str());
    if (!tmpPath) {
        LOGE("FileUtil createDirectory: strdup error!");
        return false;
    }

    char dirSep = '/';
    mode_t mode = 0755;

    for (p = tmpPath + 1; *p; p++) {
        if (*p == dirSep) {
            *p = '\0';
            if (mkdir(tmpPath, mode) && errno != EEXIST) {
                LOGE("FileUtil createDirectory: failed to create directory !");
                free(tmpPath);
                return false;
            }
            *p = dirSep;
        }
    }

    // 创建最后一层目录
    if (mkdir(tmpPath, mode) && errno != EEXIST) {
        LOGE("EngineConfiguration: failed to create directory !");
        free(tmpPath);
        return false;
    }

    free(tmpPath);
    return true;
}

bool FileUtil::createFile(const std::string &filePath) {
    FILE *file = fopen(filePath.c_str(), "r");
    if (file) {
        fclose(file);
        return true;
    }
    file = fopen(filePath.c_str(), "w+");
    if (file == NULL) {
        LOGE("FileUtil createFile: failed to create file !");
        return false;
    }
    fclose(file);
    return true;
}

bool FileUtil::createFileWriteContent(const std::string &filePath, const std::string &content) {
    FILE *file;
    file = fopen(filePath.c_str(), "w");

    if (file == NULL) {
        LOGE("FileUtil createFileWriteContent: failed to create file.");
        return false;
    }

    fprintf(file, "%s", content.c_str());
    fclose(file);
    return true;
}

bool FileUtil::deleteFile(const std::string &filePath) {
    if (remove(filePath.c_str()) != 0) {
        LOGE("FileUtil deleteFile: Failed to delete file {}.", filePath);
        return false;
    }
    return true;
}
