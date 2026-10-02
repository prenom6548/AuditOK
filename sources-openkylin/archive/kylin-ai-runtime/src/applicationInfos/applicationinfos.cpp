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

#include "applicationinfos.h"

#include <gio/gdesktopappinfo.h>

#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace appinfo {

void getXdgDataHomeDirs(std::vector<std::filesystem::path> &dataDirs) {
    const char *xdgDataHome = std::getenv("XDG_DATA_HOME");
    if (xdgDataHome) {
        dataDirs.emplace_back(std::string(xdgDataHome) + "/applications");
    } else {
        dataDirs.emplace_back(std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / "applications");
    }
}

void getXdgDataDirs(std::vector<std::filesystem::path> &dataDirs) {
    const char *xdgDataDirs = std::getenv("XDG_DATA_DIRS");
    if (xdgDataDirs) {
        std::string dirs = xdgDataDirs;
        size_t pos = 0;
        while ((pos = dirs.find(':')) != std::string::npos) {
            dataDirs.emplace_back(dirs.substr(0, pos) + "/applications");
            dirs.erase(0, pos + 1);
        }
        if (!dirs.empty()) {
            std::printf("XDG_DATA_DIRS: %s\n", dirs.c_str());
            dataDirs.emplace_back(dirs + "/applications");
        }
    } else {
        dataDirs.emplace_back("/usr/local/share/applications");
        dataDirs.emplace_back("/usr/share/applications");
    }
}

std::vector<std::filesystem::path> getXdgDataDirs() {
    std::vector<std::filesystem::path> dataDirs;
    getXdgDataHomeDirs(dataDirs);
    getXdgDataDirs(dataDirs);
    return dataDirs;
}

void findDesktopFiles(const std::filesystem::path &dir, std::vector<std::string> &appDesktopFiles) {
    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
        for (const auto &entry : std::filesystem::recursive_directory_iterator(dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".desktop") {
                appDesktopFiles.emplace_back(entry.path().string());
            }
        }
    }
}

void saveTextToFile(const std::string &textContent, const std::string &fileName) {
    FILE *file = fopen(fileName.c_str(), "a");
    if (file) {
        fprintf(file, "%s\n", textContent.c_str());
        fclose(file);
    }
    // std::printf("Saved to %s\n", fileName.c_str());
}
ApplicationInfo getAppInfoFromDesktopFile(const std::string &desktopFile) {
    GDesktopAppInfo *desktopAppinfo = g_desktop_app_info_new_from_filename(desktopFile.c_str());
    if (!desktopAppinfo) {
        return {};
    }
    if (g_desktop_app_info_has_key(desktopAppinfo, "NoDisplay") &&
        g_desktop_app_info_get_boolean(desktopAppinfo, "NoDisplay") == true) {
        g_object_unref(desktopAppinfo);
        return {};
    }
    std::string name;
    std::string comment;

    if (g_desktop_app_info_has_key(desktopAppinfo, "Name")) {
        name = g_desktop_app_info_get_locale_string(desktopAppinfo, "Name");
    }

    if (g_desktop_app_info_has_key(desktopAppinfo, "Comment")) {
        comment = g_desktop_app_info_get_locale_string(desktopAppinfo, "Comment");
    }

    g_object_unref(desktopAppinfo);
    return {desktopFile, name, comment};
}

appinfo::ApplicationInfos getAllApplicationInfos() {
    ApplicationInfos appInfos;
    std::vector<std::string> appDesktopFiles;
    std::vector<std::filesystem::path> xdgDataDirs = getXdgDataDirs();
    for (const auto &dir : xdgDataDirs) {
        std::printf("Dir: %s\n", dir.string().c_str());
        findDesktopFiles(dir, appDesktopFiles);
    }

    for (const auto &desktopFile : appDesktopFiles) {
        if (auto appInfo = getAppInfoFromDesktopFile(desktopFile); !appInfo.desktopFile.empty()) {
            appInfos.emplace_back(std::move(appInfo));
        }
    }
    return appInfos;
}

}  // namespace appinfo
