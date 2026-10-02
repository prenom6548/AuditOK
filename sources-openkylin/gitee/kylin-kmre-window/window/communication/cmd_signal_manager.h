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
#include "metatypes.h"
#include "singleton.h"

class CmdSignalManager : public QObject, public kmre::SingletonP<CmdSignalManager>
{
    Q_OBJECT

signals:
    void sendNotification(const QString &appName, const QString &pkgName, const QString &text, 
                            int winId, bool hide, bool call, const QString &title);
    void sendEventInfo(const int &eventId, const QString &pkgName);
    void sendLaunchInfo(const QString &appName, const QString &pkgName, bool result, int display_id, 
                        int width, int height, int density, bool fullscreen, bool exists, int app_resumed);
    void sendCloseInfo(const QString &pkgName);
    void sendMultiplierSwitchInfo(const QString &pkgName, int displayId, bool enable);
    void requestLaunchApp(const QString &pkgName, bool fullscreen, int width, int height, int density);
    void requestCloseApp(const QString &appName, const QString &pkgName);
    void requestFocusWindow(int winId);
    void sendClipboardInfo(const QString &content);
    void requestRunningAppList();
    void requestSendClipboardToAndroid(const QString &content);
//    void sendInputFocus(int winId, bool focus, int x, int y);
    void requestPauseAllAndroidApps();
    void requestCloseAllApps();
    void sendFocusWinId(int winId);
    void sendInputMethodRequest(int winId, const QString &pkgName, bool ret, int x, int y);
    void requestControlApp(int displayId, const QString &pkgName, int cmd, int value);
    void testLaunchInfo(const QString &pkgName);
    void testBackDekstop();
    void sendFilesList(int type, AndroidMetaList metas, int totalNum);
    void requestAddFileRecord(const QString &path, const QString &mime_type);
    void requestRemoveFileRecord(const QString &path, const QString &mime_type);
    void requestAllFiles(int type);
    void requestDragFile(const QString &filePath, const QString &pkgName, int displayId, bool hasDoubleDisplay);
    void requestChangeRotation(int id, const QString &pkgName, int width, int height, int rotation);
    void sendMediaPlayStatus(bool b);
    void initAppMultipliersList(const QString &jsonStr);
    void requestResponseInfo(int id, const QString &pkgName, const QString &category, int ret, const QString &info);
    void requestSetSystemProp(int type, const QString &propName, const QString &propValue);
    QString requestGetSystemProp(int type, const QString &propName);
    void requestUpdatePackageStatus(const QString &pkgName, int status, int type);
    void sendUpdateDDSSupportList(const QStringList &appList);
    void requestOpenUrl(const QString &url);
    void requestUpdateAppWindowSize(const QString &pkgName, int displayId, int width, int height);
    void requestUpdateDisplaySize(int displayId, int width, int height);
    void requestAnswerCall(bool answer);
    void containerEnvBooted(bool status, QString errInfo);

private:
    friend SingletonP<CmdSignalManager>;
};

