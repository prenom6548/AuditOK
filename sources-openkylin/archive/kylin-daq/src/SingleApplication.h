/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QtCore/QtCore>
#include <QtNetwork/QtNetwork>
#include <QtWidgets/QtWidgets>

#define TIME_OUT                (500)    // 500ms
class SingleApplication: public QCoreApplication
{
    Q_OBJECT
public:
    SingleApplication(int &argc, char **argv);
    bool isRunningFunction();               // 是否已经有实例在运行

private slots:
    // 有新连接时触发
    void newLocalConnection();

private:
    // 初始化本地连接
    void initLocalConnection();
    // 创建服务端
    void newLocalServer();
    // 激活窗口
    void activateWindow();
    bool m_isRunning;                // 是否已将有实例在运行
    QLocalServer *m_localServer;     // 本地socket Server
    QString m_serverName;            // 服务名称
};

#endif // SINGLEAPPLICATION_H
