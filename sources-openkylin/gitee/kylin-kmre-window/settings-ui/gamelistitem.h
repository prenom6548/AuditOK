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

#ifndef GAMELISTITEM_H
#define GAMELISTITEM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QEventLoop>
#include <QDebug>
#include <QListWidgetItem>
#include "gamelistview.h"

class GameListItem : public QWidget
{
    friend class GameListView;
    Q_OBJECT
public:
    explicit GameListItem(QWidget *parent = 0, QString pkgName="", QString appName="");
    ~GameListItem();
    QListWidgetItem *getItem();
    QString pkgName() const { return m_pkgName; }
    QString appName() const { return m_appName; }

public slots:
    void onRestoreBtnStatus(const QString &pkgName);

private:
    QHBoxLayout *m_mainLyout = nullptr;
    QString m_pkgName = nullptr;
    QString m_appName = nullptr;
    QLabel *m_pkgLabel = nullptr;
    QLabel *m_appLabel = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QListWidgetItem *m_item = nullptr;
    bool m_bSelected;
};

#endif // GAMELISTITEM_H
