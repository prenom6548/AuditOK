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
 */

#ifndef ABSTRACTDESKTOPMANAGER_H
#define ABSTRACTDESKTOPMANAGER_H

#include <QObject>
#include <QRect>
#include <QString>

class AbstractDesktopManager : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDesktopManager(QObject *parent = nullptr): QObject(parent){}
    ~AbstractDesktopManager() = default;

    virtual void removeDesktop(const QString &desktop) = 0;
    virtual void moveDesktop(const QString &srcDesktop, int destDesktopIndex) = 0;
    virtual void appendDesktop() = 0;
    virtual void setCurrentDesktop(const QString &desktop) = 0;
    virtual QString getCurrentDesktop() = 0;
    virtual int getNumberOfDesktops() = 0;
    virtual bool isCurrentDesktopShowingDesktop() = 0;
    virtual QRect getFullScreenGeometry(const QString &desktop, int screenIndex) = 0;
    virtual void setDesktopName(const QString &desktop, const QString name) = 0;
    virtual int getDesktopIndex(const QString &id) = 0;
    virtual QString getDesktopUuid(int desktopIndex) = 0;
    virtual QString getOutputUuid(int outputIndex) = 0;
    virtual bool isInitialized() = 0;

Q_SIGNALS:
    void desktopRemoved(const QString &desktop);
    void desktopAppended(const QString &desktop);
    void desktopMoved(const QString &oldDesktop, int destDesktopIndex);
    void currentDesktopChanged(const QString &oldCurrentDesktop, const QString &newCurrentDesktop);
    void workspaceManagerInitialized();
};

#endif // ABSTRACTDESKTOPMANAGER_H
