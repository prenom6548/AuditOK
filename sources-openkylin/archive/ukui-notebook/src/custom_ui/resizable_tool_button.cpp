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

#include "resizable_tool_button.h"
#include <QDebug>
ResizableToolButton::ResizableToolButton(QWidget *parent)
    : QToolButton(parent)
{
    connect(&UkuiSettingsMonitor::instance(), &UkuiSettingsMonitor::tabletModeUpdate,
                      this, &ResizableToolButton::handleTabletMode);
}

ResizableToolButton::~ResizableToolButton()
{
}

void ResizableToolButton::handleTabletMode(UkuiSettingsMonitor::TabletStatus status)
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
void ResizableToolButton::handlePADMode()
{
    if (!m_padModeSize.isEmpty()) {
        setFixedSize(m_padModeSize);
    }
}

void ResizableToolButton::handlePCMode()
{
    if (!m_pcModeSize.isEmpty()) {
        setFixedSize(m_pcModeSize);
    }
}

void ResizableToolButton::setTabletModeButtonSize(const QSize &pcMode, const QSize &padMode)
{
    m_padModeSize = padMode;
    m_pcModeSize = pcMode;
}

void ResizableToolButton::showEvent(QShowEvent *event)
{
    handleTabletMode(UkuiSettingsMonitor::instance().tabletMode());
    QToolButton::showEvent(event);
}
