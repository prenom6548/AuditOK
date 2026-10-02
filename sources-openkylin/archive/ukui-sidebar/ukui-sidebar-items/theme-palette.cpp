/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

//
// Created by hxf on 22-11-6.
//

#include "theme-palette.h"

#include <QGuiApplication>
#include <QColor>
#include <QPalette>
#include <QGSettings>

#define CONTROL_CENTER_SETTING          "org.ukui.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"

#define UKUI_STYLE_SETTING          "org.ukui.style"
#define UKUI_STYLE_NAME_KEY         "styleName"
#define UKUI_STYLE_THEME_COLOR_KEY  "themeColor"
#define UKUI_STYLE_SYSTEM_FONT_KEY  "systemFont"
#define UKUI_STYLE_SYSTEM_FONT_SIZE "systemFontSize"

static ThemePalette *globalInstance = nullptr;

ThemePalette *ThemePalette::getInstance()
{
    if (!globalInstance) {
        globalInstance = new ThemePalette(nullptr);
    }
    return globalInstance;
}
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool ThemePalette::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::ApplicationPaletteChange) {
        Q_EMIT styleColorChanged();
        return true;
    }
    return QObject::eventFilter(obj, event);
}
#endif

ThemePalette::ThemePalette(QObject *parent) : QObject(parent)
{
    initTransparency();
    initStyleSetting();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    installEventFilter(this);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(qGuiApp, &QGuiApplication::paletteChanged, this, [=] {
        Q_EMIT styleColorChanged();
    });
#endif
}

void ThemePalette::initTransparency()
{
    const QByteArray id(CONTROL_CENTER_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto *settings = new QGSettings(id, QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY) {
                m_transparency = settings->get(key).toReal();
                Q_EMIT styleColorChanged();
            }
        });

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            m_transparency = settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
        }
    }
}

void ThemePalette::initStyleSetting()
{
    const QByteArray id(UKUI_STYLE_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto *settings = new QGSettings(id, QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == UKUI_STYLE_NAME_KEY || key == UKUI_STYLE_THEME_COLOR_KEY) {
                Q_EMIT styleColorChanged();
            } else if (key == UKUI_STYLE_SYSTEM_FONT_SIZE) {
                m_fontSize = settings->get(key).toReal();
                Q_EMIT fontSizeChanged();
            } else if (key == UKUI_STYLE_SYSTEM_FONT_KEY) {
                m_font = settings->get(key).toString();
                Q_EMIT fontChanged();
            }
        });

        QStringList keys = settings->keys();
        if (keys.contains(UKUI_STYLE_SYSTEM_FONT_SIZE)) {
            m_fontSize = settings->get(UKUI_STYLE_SYSTEM_FONT_SIZE).toReal();
        }
        if (keys.contains(UKUI_STYLE_SYSTEM_FONT_KEY)) {
            m_font = settings->get(UKUI_STYLE_SYSTEM_FONT_KEY).toString();
        }
    }
}

QColor ThemePalette::paletteColor(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup) const
{
    switch (colorRole) {
        default:
        case PaletteRole::Window:
            return window(colorGroup);
        case PaletteRole::WindowText:
            return windowText(colorGroup);
        case PaletteRole::Base:
            return base(colorGroup);
        case PaletteRole::Text:
            return text(colorGroup);
        case PaletteRole::AlternateBase:
            return alternateBase(colorGroup);
        case PaletteRole::Button:
            return button(colorGroup);
        case PaletteRole::ButtonText:
            return buttonText(colorGroup);
        case PaletteRole::Light:
            return light(colorGroup);
        case PaletteRole::MidLight:
            return midLight(colorGroup);
        case PaletteRole::Dark:
            return dark(colorGroup);
        case PaletteRole::Mid:
            return mid(colorGroup);
        case PaletteRole::Shadow:
            return shadow(colorGroup);
        case PaletteRole::Highlight:
            return highlight(colorGroup);
        case PaletteRole::HighlightedText:
            return highlightedText(colorGroup);
    }
}

QColor
ThemePalette::paletteColorWithCustomTransparency(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup,
                                                 qreal alphaF) const
{
    QColor color = paletteColor(colorRole, colorGroup);
    color.setAlphaF(alphaF);
    return color;
}

QColor
ThemePalette::paletteColorWithTransparency(PaletteRole::ColorRole colorRole, PaletteRole::ColorGroup colorGroup) const
{
    QColor color = paletteColor(colorRole, colorGroup);
    color.setAlphaF(m_transparency);
    return color;
}

QColor ThemePalette::window(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Window);
}

QColor ThemePalette::windowText(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::WindowText);
}

QColor ThemePalette::text(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Text);
}

QColor ThemePalette::base(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Base);
}

QColor ThemePalette::alternateBase(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::AlternateBase);
}

QColor ThemePalette::button(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Button);
}

QColor ThemePalette::buttonText(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::ButtonText);
}

QColor ThemePalette::light(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Light);
}

QColor ThemePalette::midLight(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Midlight);
}

QColor ThemePalette::dark(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Dark);
}

QColor ThemePalette::mid(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Mid);
}

QColor ThemePalette::shadow(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Shadow);
}

QColor ThemePalette::highlight(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Highlight);
}

QColor ThemePalette::highlightedText(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::HighlightedText);
}

QColor ThemePalette::separator(PaletteRole::ColorGroup colorGroup) const
{
    return QGuiApplication::palette().color(switchColorGroup(colorGroup), QPalette::Window);
}

qreal ThemePalette::fontSize() const
{
    return m_fontSize;
}

QString ThemePalette::themeFont() const
{
    return m_font;
}

QPalette::ColorGroup ThemePalette::switchColorGroup(PaletteRole::ColorGroup colorGroup)
{
    switch (colorGroup) {
        default:
        case PaletteRole::Active:
            return QPalette::Active;
        case PaletteRole::Disabled:
            return QPalette::Disabled;
        case PaletteRole::Inactive:
            return QPalette::Inactive;
    }
}
