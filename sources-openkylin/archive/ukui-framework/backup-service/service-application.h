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
#ifndef UKUI_CONFIG_BACKUP_SERVICE_APPLICATION_H
#define UKUI_CONFIG_BACKUP_SERVICE_APPLICATION_H

#include <QObject>
#include "qtsinglecoreapplication.h"
namespace UkuiConfigBackup {
class ServiceApplication : public QtSingleCoreApplication
{
    Q_OBJECT
public:
    ServiceApplication(int &argc, char *argv[], const QString &applicationName = "ukui-config-backup-service");

protected Q_SLOTS:
    void parseCmd(QString msg, bool isPrimary);
};
}

#endif //UKUI_CONFIG_BACKUP_SERVICE_APPLICATION_H
