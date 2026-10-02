/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef IDMACTIONSMANAGER_H
#define IDMACTIONSMANAGER_H

#include <QObject>
#include <QAction>
#include "peony-core_global.h"

class QWidget;

QString truncateToByteLimit(const QString &text, int maxBytes);
namespace Peony {

class SideBarAbstractItem;
class DirectoryViewWidget;
class FMWindowIface;

class PEONYCORESHARED_EXPORT IDMActionsManager : public QObject
{
    Q_OBJECT
public:
    enum MenuType {
        SideBar,
        DirectoryView,
        Other
    };

    static IDMActionsManager *getInstance();
    static void showFileReferencedMessage(const QString &spaceName, QWidget *parent = nullptr);
    static bool addClipboardFilesToSpace(const QString &spaceName, QWidget *parent = nullptr);
    static bool deleteSpaceWithConfirmation(const QString &spaceName, QWidget *parent = nullptr);
    const QList<QAction *> constructIDMActions(SideBarAbstractItem *item = nullptr, FMWindowIface *window = nullptr, DirectoryViewWidget *view = nullptr, MenuType type = Other);

private:
    explicit IDMActionsManager(QObject *parent = nullptr);
    const QStringList getSelections(DirectoryViewWidget *view, SideBarAbstractItem *item);
    const QString getDirectoryUri(DirectoryViewWidget *view, SideBarAbstractItem *item);
    const QString getRealUriFromUri(const QString &uri);
    QStringList   m_realselectUris;

Q_SIGNALS:

};

}

#endif // IDMACTIONSMANAGER_H
