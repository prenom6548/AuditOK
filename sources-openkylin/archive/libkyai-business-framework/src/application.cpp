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

#include "application.h"

#include <getopt.h>

#include <iostream>

#include "conf.h"
#include "constant.h"
#include "error/exception.h"
#include "log.h"
#include "server/businessServer.h"

using namespace scene;

GMainLoop* loop;

Application& Application::getInstance() {
    static Application app;
    return app;
}

int Application::exec(int argc, char** argv) {
    try {
        if (!parseArgv(argc, argv)) {
            PRINT_ERROR("parse argv failed.");
            return -1;
        }

        init();

        if (func) {
            PRINT_INFO("Application exec func.\n");
            if (func()) {
                PRINT_ERROR("Application exec func failed.");
                return -1;
            }
        }

        run();
    } catch (const SystemException& e) {
        PRINT_ERROR("caught a system exception. [error_string: %s]", e.msg());
        return -1;
    } catch (...) {
        PRINT_ERROR("caught an unknown exception!");
        return -1;
    }

    return 0;
}

void Application::notifyAllConnections(const std::string& signalName, const std::string& input) {
    server->notifyAllConnections(signalName, input);
}

void Application::printHelp() {
    std::cout << "usage:\n"
              << "\t-d:\tset config file dir\n"
              << "\t-f:\tset conf filename\n"
              << "\t-s:\tset service name\n"
              << "\t-h:\tshow this page and exit server immediately\n";
}

/**
 * 从命令行获取配置文件的路径及名称，有两种方法：
 * 1、-s 服务名称；根据服务名称找固定路径下的配置文件
 * 2、-d 配置文件路径；-f 配置文件名称；指定配置文件
 */
bool Application::parseArgv(int argc, char** argv) {
    confDir = CONF_FILE_DEFAULT_DIR;
    int c;

    while (-1 != (c = getopt(argc, argv, "d:f:s:h"))) {
        switch (c) {
            case 'd':
                confDir = optarg;
                if (confDir.back() != '/') {
                    confDir.append("/");
                }
                break;
            case 'f':
                confFilename = optarg;
                break;
            case 's':
                confFilename = optarg;
                confFilename += ".json";
                break;
            default:
                printHelp();
                return false;
        }
    }
    return true;
}

void Application::init() {
    // 读取配置文件
    initConfig();
    // 设置日志等级
    initLog();

    initBusinessServer();
}

void Application::run() {
    loop = g_main_loop_new(nullptr, FALSE);
    PRINT_INFO("Server started.\n");

    g_main_loop_run(loop);
}

void Application::initConfig() {
    bool ret = Configuration::parseConfFile(confDir, confFilename);
    if (!ret) {
        PRINT_ERROR("parseConfFile [%s/%s] failed.", confDir.c_str(), confFilename.c_str());
        throw SystemException(PARSE_CONFIG_FILE_FAIL);
    }
}

void Application::initLog() {
    // 从配置文件设置日志等级，未配置设置为 info
    LogLevel logLevel = Configuration::getLogLevel();
    setLogLevel(logLevel);
}

void Application::initBusinessServer() {
    server = std::make_shared<BusinessServer>();
    server->initServer();
}

Application::~Application() {
    if (loop == nullptr) {
        return;
    }
    g_main_loop_quit(loop);
    g_main_loop_unref(loop);
}
