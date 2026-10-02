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

#ifndef __RESIZABLE_TOOL_BUTTON_H__
#define __RESIZABLE_TOOL_BUTTON_H__

#include <QToolButton>
#include <QSize>
#include "ukui_settings_monitor.h"

class ResizableToolButton : public QToolButton
{
    Q_OBJECT
public:
    ResizableToolButton(QWidget *parent = nullptr);
    ~ResizableToolButton();
    void setTabletModeButtonSize(const QSize &pcMode, const QSize &padMode);
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(UkuiSettingsMonitor::TabletStatus status);
    QSize m_pcModeSize{QSize()};
    QSize m_padModeSize{QSize()};

};
#endif // __RESIZABLE_TOOL_BUTTON_H__
