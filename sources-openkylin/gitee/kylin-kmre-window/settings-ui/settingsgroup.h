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

#ifndef SETTINGSGROUP_H
#define SETTINGSGROUP_H

#include <QFrame>
#include <QTimer>

class QVBoxLayout;
class SettingsItem;

class SettingsGroup : public QFrame
{
    Q_OBJECT

public:
    explicit SettingsGroup(QFrame *parent = 0);
    explicit SettingsGroup(const QString &title, QFrame *parent = 0);
    ~SettingsGroup();

    SettingsItem* getItem(int index);
    void insertItem(const int index, SettingsItem * item);
    void appendItem(SettingsItem * item);
    void removeItem(SettingsItem * item);

    int itemCount() const;
    void clear();

private:
    bool eventFilter(QObject *, QEvent *event);

private slots:
    void updateHeight();

private:
    QVBoxLayout *m_layout;
    QTimer *m_timer;
};

#endif // SETTINGSGROUP_H
