/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "vinointerface.h"
#include <QDebug>
#include <QProcess>

VinoInterface::VinoInterface():UKUI::ServiceObject(nullptr)
{
    mVinoGsettings = UniversalInterface::self()->gnomeVinoGsettings();
    mUkuiVinoGsettings = UniversalInterface::self()->ukuiVinoGsettings();

    mGsettingMap[kVinoViewOnlyKey] = QString("viewOnly");
    mGsettingMap[kVinoPromptKey] = QString("promptEnabled");
    mGsettingMap[kAuthenticationKey] = QString("authenticationMethods");
    mGsettingMap[kVncPwdKey] = QString("vncPassword");
    mGsettingMap[kVinoEnableKey] = QString("vinoEnabled");
    mGsettingMap[kXrdpEnableKey] = QString("xrdpEnabled");
    mGsettingMap[kEnableKey] = QString("enabled");
}

bool VinoInterface::getViewOnly()
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains("viewOnly")) {
        return mVinoGsettings->get(kVinoViewOnlyKey).toBool();
    } else {
        qCritical() << "mVinoGsettings not contains the key: " << kVinoViewOnlyKey;
    }
    return false;
}

bool VinoInterface::getPromptEnabled()
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains("promptEnabled")) {
        return mVinoGsettings->get(kVinoPromptKey).toBool();
    } else {
        qCritical() << "mVinoGsettings not contains the key: " << kVinoPromptKey;
    }
    return false;
}

QString VinoInterface::getMethod()
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains("authenticationMethods")) {
        return mVinoGsettings->get(kAuthenticationKey).toString();
    } else {
        qCritical() << "mVinoGsettings not contains the key: " << kAuthenticationKey;
    }
    return QString();
}

QString VinoInterface::getPassword()
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains("vncPassword")) {
        return mVinoGsettings->get(kVncPwdKey).toString();
    } else {
        qCritical() << "mVinoGsettings not contains the key: " << kVncPwdKey;
    }
    return QString();
}

bool VinoInterface::getVinoServerStatus()
{
    QProcess process;
    process.start("systemctl", QStringList() << "--user" << "is-active" << "vino-server.service");
    process.waitForFinished();
    bool isActive = process.readAllStandardOutput().replace("\n", "") == "active";
    return isActive;
}

bool VinoInterface::getVinoStatus()
{
    if (mVinoGsettings != nullptr && mUkuiVinoGsettings->keys().contains("vinoEnabled")) {
        return mUkuiVinoGsettings->get(kVinoEnableKey).toBool();
    } else {
        qCritical() << "mUkuiVinoGsettings not contains the key: " << kVinoEnableKey;
    }
    return false;
}

bool VinoInterface::getXrdpStatus()
{
    if (mVinoGsettings != nullptr && mUkuiVinoGsettings->keys().contains("xrdpEnabled")) {
        return mUkuiVinoGsettings->get(kXrdpEnableKey).toBool();
    } else {
        qCritical() << "mUkuiVinoGsettings not contains the key: " << kXrdpEnableKey;
    }
    return false;
}

bool VinoInterface::getEnableStatus()
{
    if (mVinoGsettings != nullptr && mUkuiVinoGsettings->keys().contains("enabled")) {
        return mUkuiVinoGsettings->get(kEnableKey).toBool();
    } else {
        qCritical() << "mUkuiVinoGsettings not contains the key: " << kEnableKey;
    }
    return false;
}

void VinoInterface::setVinoKey(const QString &key, bool value)
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains(mGsettingMap[key])) {
        return mVinoGsettings->set(key, value);
    } else {
        qCritical() << "mVinoGsettings not contains the key: " << key;
    }

    if (mUkuiVinoGsettings != nullptr && mUkuiVinoGsettings->keys().contains(mGsettingMap[key])) {
        return mUkuiVinoGsettings->set(key, value);
    } else {
        qCritical() << "mUkuiVinoGsettings not contains the key: " << key;
    }
}

void VinoInterface::setVinoKey(const QString &key, QString value)
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains(mGsettingMap[key])) {
        return mVinoGsettings->set(key, value);
    } else {
        qCritical() << "ukuiNoticeGsettings not contains the key: " << key;
    }
}

void VinoInterface::setVinoKey(const QString &key, QByteArray value)
{
    if (mVinoGsettings != nullptr && mVinoGsettings->keys().contains(mGsettingMap[key])) {
        return mVinoGsettings->set(key, value);
    } else {
        qCritical() << "ukuiNoticeGsettings not contains the key: " << key;
    }
}
