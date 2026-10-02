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
    setAttribute(Qt::WA_AlwaysShowToolTips);
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
    this->windowButtonBar()->menuButton()->hide();
}

CustomWidget::~CustomWidget()
{
}

void CustomWidget::setWidgetName(const QString& widgetName)
{
    kdk::KWidget::setWidgetName(widgetName);
    QWidget::setWindowTitle(widgetName);
}

void CustomWidget::setIcon(const QString& iconName)
{
    kdk::KWidget::setIcon(iconName);
    QWidget::setWindowIcon(QIcon::fromTheme(iconName));
}

void CustomWidget::keyPressEvent(QKeyEvent *event)
{
    if (m_isSubWindow == true) {
        switch (event->key()) {
        case Qt::Key_Escape:
            close();
            break;
        }
    }

    QWidget::keyPressEvent(event);
    return;
}

void CustomWidget::show()
{
    if (m_isSubWindow == true && m_parentWindow != nullptr) {
        centerTheWindow(this, m_parentWindow);
    }
    kdk::KWidget::show();
}

void CustomWidget::handlePADMode()
{
    // this->windowButtonBar()->maximumButton()->hide();
}

void CustomWidget::handlePCMode()
{
//    this->windowButtonBar()->maximumButton()->show();
}

void CustomWidget::showEvent(QShowEvent *event)
{
//    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
    kdk::KWidget::showEvent(event);
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
