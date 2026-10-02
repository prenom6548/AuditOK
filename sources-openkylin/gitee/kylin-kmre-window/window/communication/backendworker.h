/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef BACKENDWORKER_H
#define BACKENDWORKER_H

#include <QObject>
#include <memory>
#include <libwrapper.h>

class QTimer;

class BackendWorker : public QObject
{
    Q_OBJECT
public:
    explicit BackendWorker(QObject *parent = 0);
    ~BackendWorker();

public slots:
    bool hasEnvBootcompleted();
    QByteArray getInstalledAppListJsonStr();
    void getRunningAppList();
    void launchApp(const QString &pkgName, bool fullscreen, int width, int height, int density);
    void closeApp(const QString &appName, const QString &pkgName);
    void focusWindow(int display_id);
    void pasueAllApps();
    void setClipboardToAndroid(const QString &content);
    void controlApp(int displayId, const QString &pkgName, int cmd, int value = 0);
    void onDragFile(const QString& filePath, const QString& pkgName, int displayId, bool hasDoubleDisplay);
    void changeRotation(int displayId, const QString &pkgName, int width, int height, int rotation);
    void setSystemProp(int type, const QString &propName, const QString &propValue);
    QString getSystemProp(int type, const QString &propName);
    void updateAppWindowSize(const QString &pkgName, int displayId, int width, int height);
    void updateDisplaySize(int displayId, int width, int height);
    void answerCall(bool answer);

private:
    std::unique_ptr<LibWrapper> mKmreLibWrapper;
};

#endif // BACKENDWORKER_H
