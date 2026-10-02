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

#include "custom_widget.h"
#include "common_ui.h"
#include "global_variable.h"

CustomWidget::CustomWidget(QWidget *parent, bool isSubWindow) :
    kdk::KWidget(parent),
    m_isSubWindow(isSubWindow),
    m_parentWindow(parent)
{
    if (m_parentWindow != nullptr && m_isSubWindow == false) {
        m_isSubWindow = true;
    }

    if (m_isSubWindow == true) {
        setWindowFlags(Qt::Tool);
        setWindowModality(Qt::ApplicationModal);
    }
    setIcon(APP_ICON_NAME);

    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::tabletModeUpdate,
            this, &CustomWidget::handleTabletMode);
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::show()
{
    if (m_isSubWindow == true && m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    kdk::KWidget::show();
}

void CustomWidget::setWidgetName(const QString& widgetName)
{
    kdk::KWidget::setWidgetName(widgetName);
    QWidget::setWindowTitle(widgetName);
}

void CustomWidget::handleTabletMode(UkuiSettingsMonitor::TabletStatus status)
{
    switch (status) {
    case UkuiSettingsMonitor::TabletStatus::PCMode:
        handlePCMode();
        break;
    case UkuiSettingsMonitor::TabletStatus::PADHorizontalMode:
    case UkuiSettingsMonitor::TabletStatus::PADVerticalMode:
        handlePADMode();
        break;
    default:
        break;
    }
}

void CustomWidget::handlePADMode()
{

}

void CustomWidget::handlePCMode()
{

}

void CustomWidget::showEvent(QShowEvent *event)
{
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
    kdk::KWidget::showEvent(event);
}
