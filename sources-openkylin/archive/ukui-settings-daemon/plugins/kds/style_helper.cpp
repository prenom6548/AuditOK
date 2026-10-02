/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "style_helper.h"
#include <QApplication>
#include <QDBusMessage>
#include <QDBusConnection>
#include "usd_base_class.h"
#include "usd_global_define.h"

#define UKUI_STYLE "org.ukui.style"
const static QString s_styleName = "style-name";
const static QString s_fontSize = "system-font-size";

#define UKUI_CONTROL_CENTER_PERSONALISE "org.ukui.control-center.personalise"
const static QString s_opacity = "transparency";


Q_GLOBAL_STATIC(StyleHelper, s_styleHelper)
StyleHelper::StyleHelper(QObject *parent)
    : QObject(parent)
    , m_opacity(0.75)
    , m_fontSize(11)
    , m_isLightStyle(false)
    , m_isTabletMode(false)
{
    initStyleSettings();
}

StyleHelper *StyleHelper::instance()
{
    return s_styleHelper;
}

void StyleHelper::initStyleSettings()
{
    QByteArray styleSchemaId(UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(styleSchemaId)) {
        m_styleSettings = new QGSettings(styleSchemaId, "", this);
//        connect(m_styleSettings, &QGSettings::changed, this, &StyleHelper::slotSettingsChanged);
        if (m_styleSettings->keys().contains(s_styleName)) {
            m_isLightStyle = (m_styleSettings->get(s_styleName).toString() != QStringLiteral("ukui-dark"));
        }
        if (m_styleSettings->keys().contains(s_fontSize)) {
            m_fontSize = m_styleSettings->get(s_fontSize).toReal();

        }
    }
    QByteArray opacityId(UKUI_CONTROL_CENTER_PERSONALISE);
    if (QGSettings::isSchemaInstalled(opacityId)) {
        m_opacitySettings = new QGSettings(opacityId, "", this);
//        connect(m_opacitySettings, &QGSettings::changed, this, &StyleHelper::slotSettingsChanged);
        if (m_opacitySettings->keys().contains(s_opacity)) {
            m_opacity = m_opacitySettings->get(s_opacity).toReal();
        }
    }
}

QColor StyleHelper::getColor(QPalette::ColorRole role, QPalette::ColorGroup group)
{
    QPalette palette = QApplication::palette();
    QColor color;
    if (group == QPalette::Active) {
        switch (role) {
        case QPalette::Window:
            color = m_isLightStyle ? QColor("#FFFFFF") : QColor("#262626");
            break;
        case QPalette::Button:
            color = QColor(Qt::transparent);
            break;
        case QPalette::WindowText:
        case QPalette::ButtonText:
            color = m_isLightStyle ? QColor("#262626") : QColor("#FFFFFF");
            break;
        case QPalette::BrightText:
            color = palette.color(role);
            break;
        case QPalette::Highlight:
            color = m_isLightStyle ? QColor("#262626") : QColor("#FFFFFF");
            color.setAlphaF(0.1);
            break;
        default:
            break;
        }
    }
    return color;
}

qreal StyleHelper::getOpacity() const
{
    return m_opacity;
}

qreal StyleHelper::getFontSize() const
{
    return m_fontSize;
}

bool StyleHelper::isLightStyle() const
{
    return m_isLightStyle;
}

