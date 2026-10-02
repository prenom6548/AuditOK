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

#ifndef NETMASKITEM_H
#define NETMASKITEM_H

#include "settingsitem.h"

class QLabel;
class QSpinBox;

class NetMaskItem : public SettingsItem
{
    Q_OBJECT

public:
    explicit NetMaskItem(QFrame* parent = 0);
    void setTitle(const QString &title);
    QString getNetmask() const;
    void setWhatsThis(QWidget *w, const QString &text);
    void clear();

signals:
    void BtnEnable();

private:
    QLabel *m_titleLabel;
    QSpinBox *m_spinBox;
};

#endif // NETMASKITEM_H
