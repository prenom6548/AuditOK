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
 */

//
// Created by hxf on 22-8-15.
//

#include "color-helper.h"
#include "global-settings.h"

#include <QDebug>
#include <QtMath>

using namespace Sidebar;

static ColorHelper *globalInstance = nullptr;

ColorHelper *ColorHelper::getInstance()
{
    if (!globalInstance) {
        globalInstance = new ColorHelper(nullptr);
    }

    return globalInstance;
}

ColorHelper::ColorHelper(QObject *parent) : QObject(parent)
{
    styleChangedSlot(UKUI_STYLE_NAME_KEY);
    connect(Sidebar::GlobalSettings::globalInstance(), &Sidebar::GlobalSettings::valueChanged,
            this, &ColorHelper::styleChangedSlot, Qt::QueuedConnection);

    connect(qApp, &QApplication::paletteChanged, this, [=] {
        Q_EMIT styleColorChanged();
    });
}

void ColorHelper::styleChangedSlot(const QString &key)
{
    if (key == UKUI_STYLE_NAME_KEY || key == UKUI_STYLE_THEME_COLOR_KEY || key == CONTROL_CENTER_TRANSPARENCY_KEY) {
        // 此处由于主题框架黑名单规则，在ukui-default主题下，侧边栏的调色板为暗色，所以对于侧边栏来说，default也属于暗色主题
        m_isDarkStyle = (Sidebar::GlobalSettings::globalInstance()->getValue(UKUI_STYLE_NAME_KEY).toString() != LIGHT_STYLE);
        m_transparency = Sidebar::GlobalSettings::globalInstance()->getValue(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
        Q_EMIT styleColorChanged();
    }
}

QColor ColorHelper::pluginColor(UkuiShortcut::Color::ColorRole colorRole)
{
    QPalette palette = QApplication::palette();
    QColor color;
    switch (colorRole) {
        default:
        case UkuiShortcut::Color::BaseColor: {
//            color = palette.color(QPalette::Button);
            // 设计师：按钮的基础色为brightText+0.1的alpha
            color = palette.color(QPalette::BrightText);
            color.setAlphaF(0.1);
            break;
        }
        case UkuiShortcut::Color::HighLight: {
            color = palette.color(QPalette::Highlight);
            break;
        }
        case UkuiShortcut::Color::Success: {
            color = palette.color(QPalette::Highlight);
            break;
        }
        case UkuiShortcut::Color::Warning: {
            color = QColor("#E6A23C");
            break;
        }
        case UkuiShortcut::Color::Info: {
            color = palette.color(QPalette::Dark);
            break;
        }
        case UkuiShortcut::Color::Danger: {
            color = QColor("#FE4F4F");
            break;
        }
        case UkuiShortcut::Color::Disable: {
            color = palette.color(QPalette::Disabled, QPalette::Window);
            break;
        }
    }

    return color;
}

QColor ColorHelper::pluginColorHover(UkuiShortcut::Color::ColorRole colorRole)
{
    QColor color = pluginColor(colorRole);
    if (colorRole == UkuiShortcut::Color::BaseColor) {
        color.setAlphaF(0.2);
    } else {
        // 转换为hsv模型后，调整v分量
        qreal value = color.valueF() - (m_isDarkStyle ? -0.05 : 0.05);
        value = value > 1 ? 1 : (value < 0) ? 0 : value;
        QColor hsvColor(QColor::Hsv);
        hsvColor.setHsvF(color.hsvHueF(), color.hsvSaturationF(), value);
        color = hsvColor.toRgb();
    }
    return color;
}

QColor ColorHelper::pluginColorPressed(UkuiShortcut::Color::ColorRole colorRole)
{
    QColor color = pluginColor(colorRole);
    if (colorRole == UkuiShortcut::Color::BaseColor) {
        color.setAlphaF(0.3);
    } else {
        qreal value = color.valueF() - (m_isDarkStyle ? -0.2 : 0.2);
        value = value > 1 ? 1 : (value < 0) ? 0 : value;
        QColor hsvColor(QColor::Hsv);
        hsvColor.setHsvF(color.hsvHueF(), color.hsvSaturationF(), value);
        color = hsvColor.toRgb();
    }
    return color;
}

QColor ColorHelper::pluginColorWithCustomTransparency(UkuiShortcut::Color::ColorRole colorRole, qreal alphaF)
{
    QColor color = pluginColor(colorRole);
    color.setAlphaF(alphaF);
    return color;
}

QColor ColorHelper::pluginColorWithTransparency(UkuiShortcut::Color::ColorRole colorRole)
{
    QColor color = pluginColor(colorRole);
    color.setAlphaF(qMax(m_transparency, 0.75));
    return color;
}

bool ColorHelper::isHighLightColor(UkuiShortcut::Color::ColorRole colorRole)
{
    // 如果是暗色主题，那么图标需要进行反色
    if (m_isDarkStyle) {
        return true;
    }

    bool isHighLight;
    switch (colorRole) {
        default:
        case UkuiShortcut::Color::ColorRole::BaseColor:
        case UkuiShortcut::Color::ColorRole::Info:
        case UkuiShortcut::Color::ColorRole::Disable: {
            isHighLight = false;
            break;
        }
        case UkuiShortcut::Color::ColorRole::HighLight:
        case UkuiShortcut::Color::ColorRole::Success:
        case UkuiShortcut::Color::ColorRole::Danger: {
            isHighLight = true;
            break;
        }
    }

    return isHighLight;
}

QColor ColorHelper::separator() const
{
    QColor color = m_isDarkStyle ? QColor(255, 255, 255) :  QColor(38, 38, 38);
    color.setAlphaF(0.08);
    return color;
}

QColor ColorHelper::mask() const
{
    // 区分明暗主题
    QColor color = m_isDarkStyle ? QColor("#ffffff") :  QColor("#262626");
    color.setAlphaF(0.25);
    return color;
}

bool ColorHelper::isDarkStyle()
{
    return m_isDarkStyle;
}
