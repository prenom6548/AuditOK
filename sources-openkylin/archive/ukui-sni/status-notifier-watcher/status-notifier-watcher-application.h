/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#ifndef UKUI_SNI_STATUS_NOTIFIER_WATCHER_APPLICATION_H
#define UKUI_SNI_STATUS_NOTIFIER_WATCHER_APPLICATION_H

#include <QObject>
#include "qtsinglecoreapplication.h"
class StatusNotifierWatcherApplication : public QtSingleCoreApplication
{
    Q_OBJECT
public:
    StatusNotifierWatcherApplication(int &argc, char *argv[], const QString &applicationName = "ukui-sni-watcher");
    ~StatusNotifierWatcherApplication();

protected Q_SLOTS:
    void parseCmd(const QString& msg, bool isPrimary);
};


#endif //UKUI_SNI_STATUS_NOTIFIER_WATCHER_APPLICATION_H
