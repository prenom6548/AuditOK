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

#ifndef DESKTOPMANAGER_H
#define DESKTOPMANAGER_H

#include <QObject>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QRect>
#include <QVariantList>
#include <QString>
#include <QPointer>
#include <QMenu>
#include <QQuickItem>

#include "abstractdesktopmanager.h"

class DesktopManager : public QObject
{
    Q_OBJECT
public:
    static DesktopManager *getInstance();
    ~DesktopManager() = default;

    // AbstractDesktopManager interface
    void removeDesktop(const QString &desktop);
    void moveDesktop(const QString &srcDesktop, int destDesktopIndex);
    void appendDesktop();
    void setCurrentDesktop(const QString &desktop);
    QString getCurrentDesktop();
    int getNumberOfDesktops();
    bool isCurrentDesktopShowingDesktop();
    QRect getFullScreenGeometry(const QString &desktop, int screenIndex);
    void setDesktopName(const QString &desktop, const QString name);
    QString getDesktopUuid(int desktopIndex);
    QString getOutputUuid(int outputIndex);
    bool isInitialized();

    // 工作区右键菜单
    void openDesktopMenu(const QString &desktop, QQuickItem *item);

    // 工作区名称
    QStringList getDesktopNames() const;
    void saveDesktopNames(const QStringList &nameList);

Q_SIGNALS:
    void initialized();
    void desktopRemoved(const QString &desktop);
    void desktopAppended(const QString &desktop);
    void desktopMoved(const QString &oldDesktop, int newDesktopIndex);
    void currentDesktopChanged(const QString &oldCurrentDesktop, const QString &newCurrentDesktop);
    void workspaceManagerInitialized();
    // 工作区重命名信号，激活编辑状态
    void renameDesktopSignal(QQuickItem *item);

private:
    explicit DesktopManager(QObject *parent = nullptr);

    AbstractDesktopManager *m_impl = nullptr;
    QPointer<QMenu> m_desktopContextMenu;
};

#endif // DESKTOPMANAGER_H
