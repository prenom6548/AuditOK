/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QtDBus>
#include <QDebug>
#include <QGSettings>
#include <QVector>
#include <memory> 
#include "singleton.h"
#include "metatypes.h"
#include "communication/network/networksettings.h"

namespace kmre {

class KmreSocketConnector;
class KmreLoop;

}

class QJsonObject;
class QSessionManager;
class BackendWorker;
class CameraService;
class KmreWindowManager;
class CmdSignalManager;

class AppControlManager : public QObject, public kmre::SingletonP<AppControlManager>
{
    Q_OBJECT
public:
    void getRunningAppList();

public slots:
    // dbus method
    void setFocusWindow(int displayId);
    void closeApp(const QString &appName, const QString& pkgName, bool forceKill);
    void controlApp(int displayId, const QString &pkgName, int cmd, int value = 0);
    void sendDragFileInfo(const QString& filePath, const QString& pkgName, int displayId, bool hasDoubleDisplay);
    void sendClipboardData(const QString &content);
    void changeRotation(int id, const QString &pkgName, int width, int height, int rotation);
    void setSystemProp(int type, const QString &propName, const QString &propValue);
    QString getSystemProp(int type, const QString &propName);
    void updateAppWindowSize(const QString &pkgName, int displayId, int width, int height);
    void updateDisplaySize(int displayId, int width, int height);
    void answerCall(bool answer);

    // custom method
    void onProcessEventInfo(const int &eventId, const QString &pkgName);
    void onUpdateAppDesktopAndIcon(const QString &pkgName, int status, int type);

private slots:
    void onHandleNotification(const QString &appName, const QString &pkgName, const QString &text, 
                                int winId, bool hide, bool call, const QString &title);
    void onInitAppMultipliersList(const QString &jsonStr);
    void onResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info);
    void onOpenUrl(const QString link);
    void onSyncFiles(int type, AndroidMetaList metas, int totalNum);
    void onSyncMediaPlayStatus(bool playing);
    void onFocusDisplayById(int displayId);
    void onContainerEnvBooted(bool status, QString errInfo);

private:
    explicit AppControlManager();
    ~AppControlManager();

    void initRuntimeSocket();
    void initBackendThread();
    void initConnections();
    void initNetworkSettings();

private:
    std::shared_ptr<kmre::KmreSocketConnector> m_socketConnector;
    std::shared_ptr<kmre::KmreLoop> m_loop;

    CmdSignalManager* mCmdSignalManager;
    KmreWindowManager* mWindowManager;
    BackendWorker *m_backendWorker = nullptr;
    QThread *m_backendThread = nullptr;

    kmre::network::NetworkSettings m_networkSettings;

    friend SingletonP<AppControlManager>;

};
