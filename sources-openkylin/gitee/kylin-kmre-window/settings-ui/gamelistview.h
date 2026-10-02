/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
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

#ifndef GAMELISTVIEW_H
#define GAMELISTVIEW_H

#include "gamelistitem.h"

#include <QListWidget>
#include <QDBusReply>
#include <QtDBus>
#include <QDBusInterface>
#include <QFrame>
#include <QFile>
#include <QMap>

class GameListItem;

class GameListView : public QListWidget
{
    Q_OBJECT
public:
    GameListView(QWidget *parent = 0);
//    ~GameListView();

    void loadItem(const QString &pkgName, const QString &appName);
    QString pkgname = nullptr;
    QString appname = nullptr;

public slots:
    void removeItem(GameListItem *item);
    void removeItemByName(const QString &pkgName);
    void onResponseClearGame(const QString &pkgName);
    void refreshList();

signals:
    void requestRestoreBtnStatus(const QString &pkgName);
    void Clickeditem(const QString &pkgName,const QString &appName);

private:
    QMap<QString, GameListItem *> m_map;

};

#endif // GAMELISTVIEW_H
