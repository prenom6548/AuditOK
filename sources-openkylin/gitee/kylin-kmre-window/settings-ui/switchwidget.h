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

#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include "settingsitem.h"

class SwitchButton;
class QLabel;

class SwitchWidget : public SettingsItem
{
    Q_OBJECT

public:
    explicit SwitchWidget(QFrame *parent = 0);

    void setName(const QString& name);
    void setTitle(const QString& title);
    void setIcon(const QPixmap &icon);
    bool isChecked() const;

    void setDisabledFlag(bool b);

    QString name() {return m_name;}

public slots:
    void setChecked(const bool checked = true);

signals:
    void checkedChanged(const bool checked) const;

private:
    QLabel *m_iconLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    SwitchButton *m_switchBtn = nullptr;
    QString m_name;
};

#endif // SWITCHWIDGET_H
