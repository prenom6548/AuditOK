/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include <QObject>
#include <QVariant>
#include <QDebug>

#include "font_color_push_button.h"

FontColorPushButton::FontColorPushButton(QWidget *parent,
                                         const QColor &color,
                                         BtnType type)
    : ResizablePushButton(parent)
    , m_buttonColor(color)
{
    if (type != None) {
        setIconType(type);
    }
    setProperty("useIconHighlightEffect", 0x0);
    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::styleStatusUpdate,
                     this, &FontColorPushButton::handleStyleStatus);


}

FontColorPushButton::~FontColorPushButton()
{
}

QIcon FontColorPushButton::getButtonIcon(const QColor &buttonColor, BtnType iconType)
{
    QString type{QString()};
    switch (iconType)
    {
    case BtnType::Circle:
        type = "circle";
        break;
    case BtnType::Rect:
        type = "rect";
        break;
    default:
        break;
    }
    QString colorName = buttonColor.name();
    colorName.remove("#");
    QString iconName = QString(":/%1-%2.svg").arg(type).arg(colorName.toUpper());
    QIcon icon(iconName);
    return  icon;
}

void FontColorPushButton::setButtonColor(const QColor &color)
{
    if (color == QColor() || color == m_buttonColor) {
        return;
    }
    setIcon(getButtonIcon(color, m_IconType));
    m_buttonColor = color;
}


void FontColorPushButton::setIconType(BtnType type)
{
    if (type == m_IconType || type == None) {
        return;
    }
    setIcon(getButtonIcon(m_buttonColor, type));
    m_IconType = type;
}

void FontColorPushButton::handlePADMode()
{
    setIconType(BtnType::Circle);
    ResizablePushButton::handlePADMode();
}

void FontColorPushButton::handlePCMode()
{
    setIconType(BtnType::Rect);
    ResizablePushButton::handlePCMode();
}

void FontColorPushButton::handleDarkMode()
{
    if (m_buttonColor == QColor() || m_buttonColor.name().toUpper() == "#000000") {
        setButtonColor(QColor("#FFFFFF"));
    }
}

void FontColorPushButton::handleLightMode()
{
    if (m_buttonColor == QColor() || m_buttonColor.name().toUpper() == "#FFFFFF") {
        setButtonColor(QColor("#000000"));
    }
}

void FontColorPushButton::showEvent(QShowEvent *event)
{
    ResizablePushButton::showEvent(event);
    handleStyleStatus(UkuiSettingsMonitor::instance().styleStatus());
}

QColor FontColorPushButton::buttonColor() const
{
    return m_buttonColor;
}

void FontColorPushButton::handleStyleStatus(UkuiSettingsMonitor::StyleStatus status)
{
    switch (status) {
    case UkuiSettingsMonitor::StyleStatus::LIGHT:
        handleLightMode();
        break;
    case UkuiSettingsMonitor::StyleStatus::DARK:
        handleDarkMode();
        break;
    default:
        break;
    }
}
