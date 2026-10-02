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

#ifndef DOCKERIPWIDGET_H
#define DOCKERIPWIDGET_H

#include <QWidget>
#include <QPushButton>

#include "settingscrollcontent.h"

class SettingsGroup;
class IpItem;
class NetMaskItem;

class DockerIpWidget : public SettingScrollContent
{
    Q_OBJECT

public:
    explicit DockerIpWidget(QWidget * parent = 0);
    ~DockerIpWidget();

private:
    SettingsGroup *m_group;
    IpItem *m_ipItem;
    NetMaskItem *m_netmaskItem;
    QPushButton *m_okBtn;
    QPushButton *m_cancleBtn;
    QPushButton *m_rebootBtn;
};

#endif // DOCKERIPWIDGET_H
