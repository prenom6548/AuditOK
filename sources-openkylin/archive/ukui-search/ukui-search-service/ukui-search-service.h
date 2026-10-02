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
 * * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef UKUISEARCHSERVICE_H
#define UKUISEARCHSERVICE_H

#include <QObject>
#include <QCommandLineParser>
#include <QGSettings>
#include <QQuickView>
#include "qtsingleapplication.h"
#include "common.h"
#include "index-scheduler.h"
#include "monitor.h"
#include "index-monitor.h"

namespace UkuiSearch {

class UkuiSearchService : public QtSingleApplication
{
    Q_OBJECT
public:
    UkuiSearchService(int &argc, char *argv[], const QString &applicationName = "ukui-search-service");
    ~UkuiSearchService();

protected Q_SLOTS:
    void parseCmd(const QString& msg, bool isPrimary);
private:
    void loadMonitorWindow();
    void showMonitorState();
    static QUrl nodeUrl();

    IndexScheduler *m_indexScheduler = nullptr;
    Monitor *m_monitor = nullptr;
    QRemoteObjectHost m_qroHost;
    QQuickView *m_quickView = nullptr;
    QUrl m_qmlPath = QString("qrc:/qml/IndexMonitor.qml");

};
}
#endif // UKUISEARCHSERVICE_H
