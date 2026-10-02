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

#ifndef SETTINGSITEM_H
#define SETTINGSITEM_H

#include <QFrame>
#include <QListWidgetItem>

class SettingsItem : public QFrame
{
    Q_OBJECT

public:
    explicit SettingsItem(QWidget *parent = 0);
    QListWidgetItem* getItem();

private:
    QListWidgetItem *m_item = nullptr;
};

#endif // SETTINGSITEM_H
