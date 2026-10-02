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

#ifdef USE_SELF_CONTAINED_DEP
    #include "kwidget.h"
#else
    #include <kysdk/applications/kwidget.h>
#endif // USE_SELF_CONTAINED_DEP

#include "ukui_settings_monitor.h"

class CustomWidget : public kdk::KWidget
{
    Q_OBJECT
private:
    bool m_isSubWindow;
    QWidget *m_parentWindow;
public:
    explicit CustomWidget(QWidget *parent = nullptr, bool isSubWindow = false);
    ~CustomWidget() override;
    void setWidgetName(const QString& widgetName);
    void setIcon(const QString& iconName);
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void handleTabletMode(UkuiSettingsMonitor::TabletStatus status);
public Q_SLOTS:
    void show();
};

#endif // CUSTOM_WIDGET_H
