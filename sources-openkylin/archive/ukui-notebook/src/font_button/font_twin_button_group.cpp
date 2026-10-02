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

#include "font_twin_button_group.h"
#include "ukui_settings_monitor.h"

FontTwinButtonGroup::FontTwinButtonGroup(QWidget *parent)
    : CustomPushButtonGroup(parent)
{
    initUi();
    initConnect();
}

FontTwinButtonGroup::~FontTwinButtonGroup()
{
}

void FontTwinButtonGroup::initUi()
{
    bool isClassical =UkuiSettingsMonitor::instance().widgetThemeName() == UkuiSettingsMonitor::WidgetThemeStatus::CLASSICAL;
    frameUpdate(isClassical);
    fontSizeBtn = new FontSizePushButton(this);
    fontColorBtn = new FontColorPushButton(this);
    fontSizeBtn->setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    fontColorBtn->setTabletModeButtonSize(QSize(36, 36), QSize(48, 48));
    fontSizeBtn->setToolTip(tr("Font Size"));
    fontColorBtn->setToolTip(tr("Font Color"));
    addButton(fontSizeBtn, CustomPushButtonGroup::BtnState::Left);
    addButton(fontColorBtn, CustomPushButtonGroup::BtnState::Right);
    widgetThemeUpdate(isClassical);

}

void FontTwinButtonGroup::initConnect()
{
    QObject::connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::tabletModeUpdate,  [=](){

    });
    QObject::connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::widgetThemeNameUpdate,  [=](bool isClassical){
        widgetThemeUpdate(isClassical);
    });
}

FontSizePushButton *FontTwinButtonGroup::getFontSizeBtn()
{
    return fontSizeBtn;
}
FontColorPushButton *FontTwinButtonGroup::getFontColorBtn()
{
    return fontColorBtn;
}

void FontTwinButtonGroup::widgetThemeUpdate(bool isClassical)
{

    if (isClassical) {
        fontSizeBtn->setStyleSheet("border-top-left-radius: 0px;border-bottom-left-radius: 0px");
        fontColorBtn->setStyleSheet("border-top-right-radius: 0px;border-bottom-right-radius: 0px");
    }
    else {
        fontSizeBtn->setStyleSheet("border-top-left-radius: 6px;border-bottom-left-radius: 6px");
        fontColorBtn->setStyleSheet("border-top-right-radius: 6px;border-bottom-right-radius: 6px");
    }
}
