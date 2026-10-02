/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 * Authors: zy-yuan1 <zhangyuanyuan1@kylinos.cn>
 *
 */

#ifndef WINDOWSMANAGER_H
#define WINDOWSMANAGER_H
#include <QObject>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QRect>
#include <QVariantList>
#include <QString>

// ukui-quick
#include <window-manager.h>

//ukui-search
#include <ukui-search/application-info.h>

class WindowsManager : public QObject
{
    Q_OBJECT
public:
    static UkuiSearch::ApplicationInfo *getApplicationInfo();
    static QString desktopFileFromPid(uint pid);

    static WindowsManager *getInstance();
    ~WindowsManager() = default;

    void removeAppWindow(const QString &windowId);
    void activateAppWindow(const QString &windowId);
    void moveAppWindowToDesktop(const QString &windowId, const QString &srcDesktop, const QString &destDesktop);

    QStringList windows() const;
    bool skipSwitcher(const QString &windowId) const;
    bool hasModalChildWindow(const QString &windowId) const;
    QVariant windowIcon(const QString &wid) const;
    QString windowTitle(const QString &wid) const;
    QRect geometry(const QString &wid) const;
    bool isMinimized(const QString &wid) const;
    QStringList windowDesktops(const QString &wid) const;
    QString windowScreen(const QString &wid) const;
    int windowScreenIndex(const QString &wid) const;

Q_SIGNALS:
    void appWindowAdd(QString window);
    void appWindowRemoved(QString windowId);
    void appWindowActivated(QString windowId);
    void appWindowDesktopChanged(QString windowId);

    void titleChanged(QString wid);
    void iconChanged(QString wid);
    void geometryChanged(QString wid);

private:
    explicit WindowsManager(QObject *parent = nullptr);
    QString resolveDesktopFile(const QString &wid) const;

    UkuiQuick::WindowManager *m_windowManager = nullptr;
};

#endif // WINDOWSMANAGER_H
