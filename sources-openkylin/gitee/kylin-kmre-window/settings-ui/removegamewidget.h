/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#ifndef REMOVEGAMEWIDGET_H
#define REMOVEGAMEWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QMap>

#include "settingscrollcontent.h"
#include "gamelistitem.h"
#include "gamelistview.h"

class GameListItem;

class RemoveGameWidget : public QWidget
{
    friend class GameWidget;
    Q_OBJECT
public:
    explicit RemoveGameWidget(QWidget *parent = nullptr);
    ~RemoveGameWidget();


private:
    QVBoxLayout *m_vLayout = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_listWidget = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QMap<QString, GameListItem *> m_map;
    GameListView *m_view = nullptr;
};

#endif // REMOVEGAMEWIDGET_H
