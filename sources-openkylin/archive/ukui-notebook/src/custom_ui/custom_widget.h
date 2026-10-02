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

#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

#include <kysdk/applications/kwidget.h>
#include "ukui_settings_monitor.h"

class CustomWidget : public kdk::KWidget
{
    Q_OBJECT
private:
    bool m_isSubWindow{false};
    QWidget *m_parentWindow{nullptr};
public:
    explicit CustomWidget(QWidget *parent = nullptr, bool isSubWindow = false);
    ~CustomWidget() override;
    void setWidgetName(const QString& widgetName);
public Q_SLOTS:
    void show();
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(UkuiSettingsMonitor::TabletStatus status);
};

#endif // CUSTOM_WIDGET_H
