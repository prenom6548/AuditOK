/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef CONINFO_H
#define CONINFO_H

#include <QString>
#include <QPalette>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractItemView>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFrame>
#include "depend/kylinconnectsetting.h"

#define AUTO_CONFIG  0
#define MANUAL_CONFIG  1

enum PeapInnerType
{
    MSCHAPV2_PEAP = 0,
    MD5_PEAP,
    GTC_PEAP,
};

enum TtlsInnerType
{
    PAP = 0,
    MSCHAP,
    MSCHAPV2_EAP,
    MSCHAPV2,
    CHAP,
    MD5_EAP,
    GTC_EAP
};

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit LineEdit(QWidget *parent = nullptr);
    ~LineEdit();

    void setOobeMode(bool on);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
};

class ConInfo {
public:

    QString strConName;
    QString strConType = "802-3-ethernet";
    QString strMac;
    QString strBandWidth;
    QString strDynamicIpv4;
    QString strDynamicIpv6;
    QString strDynamicIpv4Dns;
    bool    isAutoConnect = false;

    KyIpConfigType ipv4ConfigType = CONFIG_IP_DHCP;
    QString strIPV4Address;
    QString strIPV4NetMask;
    QString strIPV4FirDns;
    QString strIPV4SecDns;
    QString strIPV4GateWay;

    KyIpConfigType ipv6ConfigType = CONFIG_IP_DHCP;
    QString strIPV6Address;
    int iIPV6Prefix;
    QString strIPV6FirDns;
    QString strIPV6SecDns;
    QString strIPV6GateWay;
};

void setFramePalette(QFrame *widget, QPalette &pal);

QPalette lightPalette(QWidget *widget);

#endif // CONINFO_H
