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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace scene {
class BusinessServer;

using Func = std::function<int()>;

class Application {
public:
    static Application& getInstance();
    int exec(int argc, char* argv[]);
    void notifyAllConnections(const std::string& signalName, const std::string& input);
    void setFunc(Func f) { func = std::move(f); }

private:
    Application() = default;
    ~Application();
    bool parseArgv(int argc, char* argv[]);
    void init();
    void run();

    static void printHelp();
    void initConfig();
    void initLog();
    void initBusinessServer();

private:
    ::std::string confDir;
    ::std::string confFilename;
    std::shared_ptr<BusinessServer> server;
    Func func;
};
}  // namespace scene
