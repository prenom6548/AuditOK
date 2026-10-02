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
#include <QDebug>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <gsettingmonitor.h>   // SDK gsetting
#include "external.h"

#define KYLIN_FONT_VIEWER_GSETTINGS "org.kylin.font.viewer.gsettings"
#define COLLECT_FONT_KEY "collect-font"
#define TIPS_FONT_KEY "setting-font-tips"
#define THEME_GSETTINGS "org.ukui.style"
#define APPLY_FONT_KEY "system-font"
#define KYLIN_CONTROL_CENTER_GSETTINGS "org.ukui.control-center"
#define FONTS_LIST "fonts-list"
#define KYLIN_GTK_GSTTINGS "org.mate.interface"
#define FONT_NAME "font-name"
#define DOC_FONT_KEY "document-font-name"

External::External()
{
    if (QGSettings::isSchemaInstalled(KYLIN_FONT_VIEWER_GSETTINGS)) {
        this->m_gsettings = new QGSettings(KYLIN_FONT_VIEWER_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(THEME_GSETTINGS)) {
        this->m_themeGsettings = new QGSettings(THEME_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create theme gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(KYLIN_CONTROL_CENTER_GSETTINGS)) {
        this->m_contolGsettings = new QGSettings(KYLIN_CONTROL_CENTER_GSETTINGS);
    } else {
        qCritical() << "Error : External , External , create control gsettings fail";
    }

    if (QGSettings::isSchemaInstalled(KYLIN_GTK_GSTTINGS)) {
        this->m_gtkGsettings = new QGSettings(KYLIN_GTK_GSTTINGS);
    } else {
        qCritical() << "Error : External , External , create GTK gsettings fail";
    }
}

External::~External()
{
    if (this->m_gsettings != nullptr) {
        delete this->m_gsettings;
    }
    if (this->m_themeGsettings != nullptr) {
        delete this->m_themeGsettings;
    }
    if (this->m_contolGsettings != nullptr) {
        delete this->m_contolGsettings;
    }
    if (this->m_gtkGsettings != nullptr) {
        delete this->m_gtkGsettings;
    }
}


bool External::applyFont(QString family)
{
    qDebug() << "apply font family " << family;
    if (!setFontIntoList(family)) {
        return false;
    }

    double sysFontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    QString fontStyle = family + " " + QString::number(sysFontSize, 'f', 1);

    this->m_themeGsettings->set(APPLY_FONT_KEY, family);// 改变主题字体
    this->m_gtkGsettings->set(FONT_NAME, fontStyle);    // 改变GTK的字体
    this->m_gtkGsettings->set(DOC_FONT_KEY, fontStyle); // 改变GTK的文档字体

    const int fontSize = sysFontSize;
    QDBusMessage message =QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "slotFontChange");
    QList<QVariant> args;
    args.append(fontSize);
    args.append(family);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message); // 使用dbus改变GTK标题栏的字体
    return true;
}

QStringList External::getFontFromControl()
{
    QStringList fontList;
    fontList.clear();

    if (this->m_contolGsettings == nullptr) {
        qCritical() << "Error : gsettings fail , org.ukui.control-center";
        return fontList;
    }
    fontList = this->m_contolGsettings->get(FONTS_LIST).toStringList();
    return fontList;
}

bool External::setFontIntoList(QString font)
{
    QStringList fontList;
    fontList.clear();
    fontList = getFontFromControl();
    if (fontList.size() == 0) {
        return false;
    }
    if (!fontList.contains(font)) {
        fontList.append(font);
    }
    m_contolGsettings->set(FONTS_LIST, fontList);

    return true;
}

QString External::isUsingFont()
{
    QString font = "";
    if (this->m_themeGsettings == nullptr) {
        qCritical() << "Error : gsettings fail , org.ukui.style";
        return font;
    }
    font = this->m_themeGsettings->get(APPLY_FONT_KEY).toString();
    qDebug() << "using font name: " << font;

    return font;
}

bool External::removeFontFormContro(QString font)
{
    QStringList fontList;
    fontList.clear();
    fontList = getFontFromControl();
    if (fontList.size() == 0) {
        return false;
    }
    if (fontList.contains(font)) {
        fontList.removeOne(font);
    }
    m_contolGsettings->set(FONTS_LIST, fontList);
    return true;
}
