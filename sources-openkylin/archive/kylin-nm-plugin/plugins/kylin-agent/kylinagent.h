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

#ifndef KYLINAGENT_H
#define KYLINAGENT_H

#include "kylinsecretagent.h"
//#include <QObject>

#if defined(LIBKYLIN_KEYRING_LIBRARY)
#  define LIBKYLIN_KEYRING_LIBRARY Q_DECL_EXPORT
#else
#  define LIBKYLIN_KEYRING_LIBRARY Q_DECL_IMPORT
#endif

class LIBKYLIN_KEYRING_LIBRARY KylinAgent : public QObject
{
    Q_OBJECT

public:
    explicit KylinAgent(QObject *parent = nullptr);
    ~KylinAgent();
    void startKylinAgent();
    void setParentWidget(QWidget *widget){
        pWidget = widget;
    }

    // 供前端调用的回传函数
    void provideWirelessSecret(const QString &requestId, const QString &password, bool autoConnect);
    void provideEnterpriseSecret(const QString &requestId, const QVariantMap &secretMap);
    void cancelSecretRequest(const QString &requestId);

Q_SIGNALS:
    void requestWirelessSecret(const QString &ssid, const QString &password, const QString &devName,
                               const QString &settingName, uint flags, const QString &requestId);
    void requestEnterpriseSecret(const QString &ssid, const QString &devName,
                                 const QString &settingName, uint flags,
                                 const QVariantMap &currentSettings,
                                 const QString &requestId);

private:
    KylinSecretAgent *m_secretAgnet;
    QWidget* pWidget = nullptr;
};


#endif // KYLINAGENT_H
