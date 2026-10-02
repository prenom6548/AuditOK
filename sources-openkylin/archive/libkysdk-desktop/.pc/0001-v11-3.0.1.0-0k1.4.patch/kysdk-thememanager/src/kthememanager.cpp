/*
 * libkysdk-thememanager's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "kthememanager.h"
#include <QGSettings>
#include <QVariant>
#include <QDebug>

// GTK主题
#define MATE_INTERFACE_SCHEMA   "org.mate.interface"
#define MODE_GTK_KEY            "gtk-theme"
#define ICON_GTK_KEY            "icon-theme"

#define FONT_DOC_KEY       "document-font-name"  // 用于阅读文档的默认字体的名称
#define FONT_GTK_KEY       "font-name"           // gkt+使用的默认字体
#define MONOSPACE_FONT_KEY "monospace-font-name" // 用于终端等处的等宽字体

// QT主题
#define STYLE_SCHEMA     "org.ukui.style"
#define MODE_QT_KEY      "style-name"
#define ICON_QT_KEY      "icon-theme-name"
#define WIDGET_QT_KEY    "widget-theme-name"
#define COLOR_QT_KEY     "theme-color"

#define UKCC_SCHEMA        "org.ukui.control-center"
#define UKCC_FONTS_EKY     "fonts-list"
#define SYSTEM_FONT_KEY         "system-font"
#define SYSTEM_FONT_SIZE_KEY    "system-font-size"

// UKCC 全局主题
#define GLOBAL_SCHEMA    "org.ukui.globaltheme.settings"
#define GLOBAL_NAME_KEY  "global-theme-name"
#define ISMODIFY_KEY     "is-modified"

// 透明度设置
#define PERSONALSIE_SCHEMA        "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY      "transparency"
#define PERSONALSIE_BLURRY_KEY    "blurry"
#define PERSONALSIE_EFFECT_KEY    "effect"
#define PERSONALSIE_SAVE_TRAN_KEY "save-transparency"

// 自动主题设置
#define AUTO_SCHEMA               "org.ukui.SettingsDaemon.plugins.color"
#define COLOR_KEY_AUTO_THEME      "theme-schedule-automatic"

namespace kdk {

QStringList styleList = {"ukui-dark", "ukui-default", "ukui-light"};

class KThemeManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KThemeManager)

public:
    KThemeManagerPrivate(QObject *parent = nullptr);
    virtual ~KThemeManagerPrivate();

public:
    QGSettings *gtkSettings           = nullptr;
    QGSettings *styleSettings         = nullptr;
    QGSettings *mGlobalSettings       = nullptr;
    QGSettings *personalsieSettings   = nullptr;

    KThemeManager *q_ptr;
};
}

kdk::KThemeManager::KThemeManager(QObject *parent)
    : QObject(parent),
      d_ptr(new KThemeManagerPrivate(this))
{
    Q_D(KThemeManager);

    if(d->styleSettings)
    {
        connect(d->styleSettings, &QGSettings::changed, [=](const QString &key){
            if (key == "styleName"){
                emit changed(MODE_QT_KEY ,d->styleSettings->get(MODE_QT_KEY));
            } else if (key == "systemFont"){
                emit changed(SYSTEM_FONT_KEY ,d->styleSettings->get(SYSTEM_FONT_KEY));
            } else if (key == "systemFontSize"){
                emit changed(SYSTEM_FONT_SIZE_KEY ,d->styleSettings->get(SYSTEM_FONT_SIZE_KEY));
            } else if (key == "themeColor"){
                emit changed(COLOR_QT_KEY ,d->styleSettings->get(COLOR_QT_KEY));
            }
        });
    }

    if (d->personalsieSettings){
        connect(d->personalsieSettings, &QGSettings::changed, [=](const QString &key){
            if (key == "effect"){
                emit changed(PERSONALSIE_EFFECT_KEY ,d->personalsieSettings->get(PERSONALSIE_EFFECT_KEY));
            } else if (key == "transparency"){
                emit changed(PERSONALSIE_TRAN_KEY ,d->personalsieSettings->get(PERSONALSIE_TRAN_KEY));
            }
        });
    }
}

kdk::KThemeManager::~KThemeManager()
{

}

QString kdk::KThemeManager::getUkccGlobalTheme()
{
    Q_D(KThemeManager);

    return d->mGlobalSettings ? d->mGlobalSettings->get(GLOBAL_NAME_KEY).toString() : "";
}

QStringList kdk::KThemeManager::getSupportSystemStyle()
{
    return styleList;
}

QString kdk::KThemeManager::getSystemThemeColor()
{
    Q_D(KThemeManager);

    return d->styleSettings ? d->styleSettings->get(COLOR_QT_KEY).toString() : "";
}

QString kdk::KThemeManager::getSystemFont()
{
    Q_D(KThemeManager);

    return d->styleSettings ? d->styleSettings->get(SYSTEM_FONT_KEY).toString() : "";
}

int kdk::KThemeManager::getSystemFontSize()
{
    Q_D(KThemeManager);

    return d->styleSettings ? d->styleSettings->get(SYSTEM_FONT_SIZE_KEY).toInt() : -1;
}

bool kdk::KThemeManager::setSystemStyle(const QString &style)
{
    Q_D(KThemeManager);

    if (!styleList.contains(style) ||!d->styleSettings)
        return false;

    QString tmpMode = "ukui-white";
    if ("ukui-dark" == style) {
        tmpMode = "ukui-black";
    } else if ("ukui-light" == style) {
        tmpMode = "ukui-white";
    }

    if (!d->gtkSettings->trySet(MODE_GTK_KEY, tmpMode))
    {
        return false;
    }
    if (!d->styleSettings->trySet(MODE_QT_KEY, style))
    {
        return false;
    }
    return true;
}

bool kdk::KThemeManager::setSystemFont(const QString &font)
{
    Q_D(KThemeManager);

    if(!d->styleSettings)
        return false;

    QGSettings ukccSetting(UKCC_SCHEMA);
    QStringList fonts = ukccSetting.get(UKCC_FONTS_EKY).toStringList();
    if (!fonts.contains(font, Qt::CaseSensitive))
        return false;

    return d->styleSettings->trySet(SYSTEM_FONT_KEY, font);
}

bool kdk::KThemeManager::setSystemFontSize(int size)
{
    Q_D(KThemeManager);

    if (!d->styleSettings)
        return false;

    return d->styleSettings->trySet(SYSTEM_FONT_SIZE_KEY, QString::number(size));
}

bool kdk::KThemeManager::setSystemThemeColor(const QString &color)
{
    Q_D(KThemeManager);

    QStringList colorList = {"default","daybreakBlue","jamPurple","magenta","sunRed","sunsetOrange","dustGold","polarGreen"};
    if (!colorList.contains(color) || !d->styleSettings)
        return false;

    return d->styleSettings->trySet(COLOR_QT_KEY, color);
}

bool kdk::KThemeManager::setPanelSpecialEffect(bool effect)
{
    Q_D(KThemeManager);

    if (!d->personalsieSettings)
        return false;

    return d->personalsieSettings->trySet(PERSONALSIE_EFFECT_KEY, effect);
}

bool kdk::KThemeManager::setTransparency(double tran)
{
    Q_D(KThemeManager);

    if (!d->personalsieSettings)
        return false;

    return d->personalsieSettings->trySet(PERSONALSIE_TRAN_KEY, tran);
}

kdk::KThemeManagerPrivate::KThemeManagerPrivate(QObject *parent)
    : QObject(parent),
      q_ptr((KThemeManager*)parent)
{
    gtkSettings         = new QGSettings(MATE_INTERFACE_SCHEMA,   QByteArray(), this);
    styleSettings       = new QGSettings(STYLE_SCHEMA,  QByteArray(), this);
    personalsieSettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);

    if (QGSettings::isSchemaInstalled(GLOBAL_SCHEMA)) {
        mGlobalSettings = new QGSettings(GLOBAL_SCHEMA, QByteArray(), this);
    }
}

kdk::KThemeManagerPrivate::~KThemeManagerPrivate()
{
}

#include "kthememanager.moc"
#include "moc_kthememanager.cpp"
