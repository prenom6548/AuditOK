/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef UKCCSESSIONSERVER_H
#define UKCCSESSIONSERVER_H

#include <ukui-service-interface/serviceobject.h>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QVariantMap>
#include <QCoreApplication>
#include <QDBusContext>
#include <QScopedPointer>
#include <QDBusVariant>
#include <QFileSystemWatcher>
#include <QSettings>
#include <kysdk/kysdk-system/libkysysinfo.h>

class ukccSessionServer : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.interface")
public:
    explicit ukccSessionServer();

private:
    QMap<QString, QVariant> getJsonInfo(const  QString &confFile);
    QMap<QString, QVariant> getUSDJsonInfo(const  QString &confFile);
    void monitoFileChanged();
    QString exchangeModuleName(QString originName);

private:
    QString mScreenMode;
    QString mUsdJsonFile = "";

Q_SIGNALS:
    void configChanged();
    void screenChanged(QString status);
    void searchItemsAdd(QVariantMap addItems);
    void searchItemsDelete(QVariantMap deleteItems);

public Q_SLOTS:
    void exitService();
    void        ReloadSecurityConfig();
    QVariantMap getModuleHideStatus();
    QVariantMap getSearchItems();
    QString     GetSecurityConfigPath();
    QString getScreenMode();
    void setScreenMode(QString screenMode);
};

#endif // UKCCSESSIONSERVER_H
