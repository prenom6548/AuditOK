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
#include "customglobaltheme.h"

#include <QGSettings>
#include <QSettings>
#include <QStandardPaths>
#include <QTextCodec>
#include <QThread>

#include <QMetaMethod>

#include <QDebug>

#define USER_THEME_DIR QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).arg("globaltheme/")

CustomGlobalTheme::CustomGlobalTheme(QObject *parent) : GlobalTheme(parent)
{
    if (QGSettings::isSchemaInstalled("org.ukui.globaltheme.settings")) {
        gsettings = new QGSettings("org.ukui.globaltheme.settings", "/org/ukui/globaltheme/settings/", this);
        connect(gsettings, &QGSettings::changed, this, &CustomGlobalTheme::onGlobalSettingsChanged);
    }

    QString path = USER_THEME_DIR + "custom.conf";
    qsettings = new QSettings(path, QSettings::NativeFormat, this);
    qsettings->setIniCodec(QTextCodec::codecForName("utf-8"));
}

QString CustomGlobalTheme::getThemeName()
{
    return "custom";
}

QString CustomGlobalTheme::getLocaleThemeName()
{
    return tr("Custom");
}

bool CustomGlobalTheme::getSupportLightDarkMode()
{
    return qsettings->value("getSupportLightDarkMode", false).toBool();
}

QString CustomGlobalTheme::getDefaultLightDarkMode()
{
    return qsettings->value("getDefaultLightDarkMode", "light").toString();
}

QString CustomGlobalTheme::getWidgetStyleName()
{
    return qsettings->value("getWidgetStyleName", "fashion").toString();
}

QString CustomGlobalTheme::getQtLightThemeName()
{
    return qsettings->value("getQtLightThemeName", "ukui-light").toString();
}

QString CustomGlobalTheme::getQtDarkThemeName()
{
    return qsettings->value("getQtDarkThemeName", "ukui-dark").toString();
}

QString CustomGlobalTheme::getQtMixedThemeName()
{
    return qsettings->value("getQtMixedThemeName", "ukui-default").toString();
}

QString CustomGlobalTheme::getQtAccentRole()
{
    return qsettings->value("getQtAccentRole", "daybreakBlue").toString();
}

QString CustomGlobalTheme::getGtkThemeName()
{
    return qsettings->value("getGtkThemeName", "ukui-white").toString();
}

QString CustomGlobalTheme::getGtkLightThemeName()
{
    return qsettings->value("getGtkLightThemeName", "ukui-white").toString();
}

QString CustomGlobalTheme::getGtkDarkThemeName()
{
    return qsettings->value("getGtkDarkThemeName", "ukui-black").toString();
}

QString CustomGlobalTheme::getGtkAccent()
{
    return qsettings->value("getGtkAccent", "#3790FA").toString();
}

QString CustomGlobalTheme::getIconThemeName()
{
    return qsettings->value("getIconThemeName", "ukui-icon-theme-default").toString();
}

QString CustomGlobalTheme::getCursorThemeName()
{
    return qsettings->value("getCursorThemeName", "DMZ-White").toString();
}

QString CustomGlobalTheme::getSoundThemeName()
{
    return qsettings->value("getSoundThemeName", "Light-Seeking").toString();
}

QString CustomGlobalTheme::getWallPaperPath()
{
    return qsettings->value("getWallPaperPath", "/usr/share/backgrounds/ubuntukylin-default-settings.jpg").toString();
}

bool CustomGlobalTheme::getSupportBlur()
{
    return qsettings->value("getSupportBlur").toBool();
}

bool CustomGlobalTheme::getBlurEnabled()
{
    return qsettings->value("getBlurEnabled", true).toBool();
}

bool CustomGlobalTheme::getSupportTransparency()
{
    return qsettings->value("getSupportTransparency").toBool();
}

int CustomGlobalTheme::getTransparencyBlur()
{
    return qsettings->value("getTransparencyBlur", 65).toInt();
}

int CustomGlobalTheme::getTransparencyNoBlur()
{
    return qsettings->value("getTransparencyNoBlur", 90).toInt();
}

bool CustomGlobalTheme::getSupportAnimation()
{
    return qsettings->value("getSupportAnimation").toBool();
}

int CustomGlobalTheme::getAnimationDuration()
{
    return qsettings->value("getAnimationDuration", 150).toInt();
}

bool CustomGlobalTheme::getSupportRadius()
{
    return qsettings->value("getSupportRadius").toBool();
}

QString CustomGlobalTheme::getRadiusValue()
{
    return qsettings->value("getRadiusValue", "12,8").toString();
}

void CustomGlobalTheme::loadThumbnail()
{
    QThread *t = new QThread;
    m_customThumbnailer = new CustomThumbnailer;
    m_customThumbnailer->moveToThread(t);
    connect(t, &QThread::started, m_customThumbnailer, &CustomThumbnailer::startLoad);
    connect(m_customThumbnailer, &CustomThumbnailer::thumbnailUpdated, this, &CustomGlobalTheme::updateThumbnail, Qt::BlockingQueuedConnection);
    connect(t, &QThread::finished, m_customThumbnailer, &CustomThumbnailer::deleteLater);
    t->start();
}

CustomGlobalTheme::~CustomGlobalTheme() {
    if (m_customThumbnailer) {
        delete m_customThumbnailer;
        m_customThumbnailer = nullptr;
    }
}

QPixmap CustomGlobalTheme::getThumbnail()
{
    return pixmap;
}

void CustomGlobalTheme::updateThumbnail(const QPixmap &pixmap)
{
    qDebug()<<"custom thumbnail loaded"<<pixmap;
    this->pixmap = pixmap;
    Q_EMIT thumbnailLoaded();
}

bool CustomGlobalTheme::isModified()
{
    if (!gsettings) {
        return false;
    }
    return gsettings->get("idModified").toBool();
}

void CustomGlobalTheme::loadFromOtherGlobalTheme(GlobalTheme *other)
{
    if (!other) {
        qCritical()<<"load from other global theme failed, globaltheme is invalid!";
        return;
    }

    if (other->getThemeName() == "custom")
        return;

    int methodCount = other->metaObject()->methodCount();
    for (int i = 0; i < methodCount; i++) {
        auto method = other->metaObject()->method(i);
        if (method.parameterCount() > 0)
            continue;
        QString var;
        if (other->metaObject()->invokeMethod(other, method.name().constData(), Q_RETURN_ARG(QString, var))) {
            qsettings->setValue(method.name().constData(), var);
        } else {
            bool var;
            if (other->metaObject()->invokeMethod(other, method.name().constData(), Q_RETURN_ARG(bool, var))) {
                qsettings->setValue(method.name().constData(), var);
            }
        }
    }
    qsettings->sync();
}

void CustomGlobalTheme::updateValue(const QString &functionName, const QVariant &value)
{
    qsettings->setValue(functionName, value);
    qsettings->sync();
}

void CustomGlobalTheme::onGlobalSettingsChanged(const QString &key)
{
    if (key == "isModified") {

    } else if (key == "globalThemeName") {

    }
}

CustomThumbnailer::CustomThumbnailer(QObject *parent) : QObject(parent)
{

}

void CustomThumbnailer::startLoad()
{
    QPixmap pixmap("/usr/share/config/globaltheme/custom-preview.png");
    thumbnailUpdated(pixmap);
}
