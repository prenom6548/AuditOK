/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#include "kylin-ai/common/common.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>

static const char *AI_SUBSYSTEM_PROCESS = "kylin-ai-runtime";
static const char *AI_SUBSYSTEM_PACKAGE = "kylin-ai-subsystem";
static const char *AI_SUBSYSTEM_FILEPATH = "/tmp/kylin-ai-subsystem-plugin";

namespace {

bool hasProcessByName(const std::string &namePart) {
    const std::string command = "ps ax | grep " + namePart + " | grep -v grep";

    // 通过管道获取命令执行结果
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        std::cerr << "popen() failed!" << std::endl;
        return false;
    }

    char buffer[128];
    const bool found = fgets(buffer, sizeof(buffer), pipe.get()) != nullptr;

    return found;
}

bool isPkgInstalledAndReboot() {
    std::ifstream file(AI_SUBSYSTEM_FILEPATH);
    bool fileExist = file.is_open();
    file.close();
    if (fileExist) {
        fprintf(stderr,
                "KysdkAiCommon: AI package installed but not restarted!\n");
        return false;
    }

    std::string command = "LC_ALL=C /usr/bin/dpkg -s kylin-ai-subsystem 2>/dev/null";
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        fprintf(stderr, "KysdkAiCommon: popen error!\n");
        return false;
    }

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("Status: install ok installed") != std::string::npos) {
            pclose(pipe);
            pipe = nullptr;
            return true;
        }
    }
    pclose(pipe);
    pipe = nullptr;
    return false;
}

}  // namespace

bool is_ai_subsystem_inited() {
    return hasProcessByName(AI_SUBSYSTEM_PROCESS) && isPkgInstalledAndReboot();
}
