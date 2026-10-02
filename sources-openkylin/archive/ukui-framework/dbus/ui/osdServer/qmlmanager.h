/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QMLMANAGER_H
#define QMLMANAGER_H

#include <QObject>
#include <QBuffer>

#include <QTimer>
#include <QString>
#include <memory>

#include <QDateTime>

// [新增] 引入 D-Bus 相关头文件
#include <QDBusInterface>
#include <QDBusConnection>

#include "osd_string.h"
#include "datamanager.h"
#include "qgsettings.h"

#include "osdbasicwindow.h"

#define OSD_MINIMUM_SHOWTIME 500
#define OSD_QML_PATH(name) QString("qrc:/ukui_res/qml/%1").arg(name)

#define DBUS_USD_SERVICE    "com.kylin.ukui.SettingsDaemon"
#define DBUS_USD_PATH       "/inputEventMonitor"
#define DBUS_USD_INTERFACE  "com.kylin.ukui.SettingsDaemon.interface"
#define DBUS_USD_MOTHOD     "MouseMoved"


class QmlManager : public QObject{

    Q_OBJECT

public:
    explicit QmlManager(QObject* parent = nullptr);
    static QmlManager *instance();

    void onShowIcon(const QString& sourceName);

    void onShowBar(const QString& sourceName, double currentData, double maxData);

public slots:
    void onHideAnimationFinished();
    // [新增] 处理 D-Bus 鼠标移动信号的槽函数
    void onDbusMouseMoved(int dx, int dy);

private:
    DataManager* m_pDataManager = nullptr;

    std::unique_ptr<OsdBasicWindow> m_osdBasic = nullptr;

    QTimer *m_osdHideTimer = nullptr;

    QString m_lastUrl;

    QDateTime m_lastShowOsdTime;

private:
    void initHideTimer();

    void initDataManager();

    void showOsdView(int width, int height, const QString& url);

    void createOsdView(int width, int height, const QString& url);

    bool isNeedCreateOsdView(const QString& url);

    void connectMouseMovedSignal();
    void disconnectMouseMovedSignal();

signals:
    void initIconData(const QString& sourceName);
    void initBarData(const QString& sourceName, double currentData, double maxData);

    // [新增] 通知 QML 开始执行渐隐动画
    void requestHideAnimation();
};

#endif // QMLMANAGER_H
