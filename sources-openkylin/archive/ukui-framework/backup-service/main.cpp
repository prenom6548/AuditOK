/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <QDebug>
#include "service-application.h"
#include "log-utils.h"
int main(int argc, char *argv[])
{
    LogUtils::initLogFile("ukui-config-backup-service");
    qInstallMessageHandler(LogUtils::messageOutput);
    UkuiConfigBackup::ServiceApplication app(argc, argv);
    if(app.isRunning()) {
        qDebug() << "Service Application is already running.";
        return 0;
    }
    return app.exec();
}