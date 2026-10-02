/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include <cstdlib>
#include <unistd.h>
#include <syslog.h>
#include <QDir>
#include <QDebug>
#include "log-utils.h"
#include "ukui-search-dir-manager-dbus.h"

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("HOME")) {
        qCritical() << "HOME is not set, exit dir manager.";
        syslog(LOG_ERR, "HOME is not set, exit dir manager.\n");
        return EXIT_FAILURE;
    }

    // home 目录可能在服务拉起后才建出来，最多等 5 秒
    constexpr int kMaxWaitSec = 5;
    for (int i = 0; i < kMaxWaitSec && !QDir(QDir::homePath()).exists(); ++i) {
        ::sleep(1);
    }
    if (!QDir(QDir::homePath()).exists()) {
        qCritical() << "Home directory does not exist after waiting, exit dir manager.";
        syslog(LOG_ERR, "Home directory does not exist after waiting, exit dir manager.\n");
        return EXIT_FAILURE;
    }

    // Output log to file
    UkuiSearch::LogUtils::initLogFile("ukui-search-service-dir-manager");
    qInstallMessageHandler(UkuiSearch::LogUtils::messageOutput);
    UkuiSearchDirManagerDbus dirDbus(argc, argv, "ukui-search-service-dir-manager");

    if (dirDbus.isRunning()) {
        return 0;
    }
    if (!dirDbus.initializationSucceeded()) {
        return EXIT_FAILURE;
    }
    return dirDbus.exec();
}
