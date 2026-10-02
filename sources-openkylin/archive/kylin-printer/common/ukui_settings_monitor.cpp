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

//#include "gsettings_monitor.h"
#include "horizontalorverticalmode.h"
#include "ukui_settings_monitor.h"
#include "QTimer"
UkuiSettingsMonitor::UkuiSettingsMonitor()
{
//    d_ptrGsetting = new GsettingsMonitor();
    d_ptrDbus = new HorizontalOrVerticalMode();

//    connect(d_ptrGsetting, &GsettingsMonitor::transparencyUpdate, this, &UkuiSettingsMonitor::transparencyUpdate);
//    connect(d_ptrGsetting, &GsettingsMonitor::styleStatusUpdate, this, [=](GsettingsMonitor::StyleStatus status){
//        emit styleStatusUpdate(StyleStatus(status));
//    });
    connect(d_ptrDbus,     &HorizontalOrVerticalMode::RotationSig, this, [=](deviceMode mode){
        emit tabletModeUpdate(TabletStatus(mode));
    });

}

UkuiSettingsMonitor::~UkuiSettingsMonitor()
{
//    delete d_ptrGsetting;
    delete d_ptrDbus;
}

//int UkuiSettingsMonitor::transparency() const
//{
//    return d_ptrGsetting->transparency();
//}

//UkuiSettingsMonitor::StyleStatus UkuiSettingsMonitor::styleStatus() const
//{
//    return StyleStatus(d_ptrGsetting->styleStatus());
//}

UkuiSettingsMonitor::TabletStatus UkuiSettingsMonitor::tabletMode() const
{
    return TabletStatus(d_ptrDbus->defaultModeCapture());
}

void UkuiSettingsMonitor::startTest(TabletStatus status)
{
    static int cnt = 0;
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, [=](){
        if (status != None) {
            emit this->tabletModeUpdate(status);
            return ;
        }
        cnt++;

        if (cnt % 2 == 0) {
            emit this->tabletModeUpdate(TabletStatus::PCMode);
        } else {
            emit this->tabletModeUpdate(TabletStatus::PADHorizontalMode);
        }
    });
    timer->start(1000);

}

