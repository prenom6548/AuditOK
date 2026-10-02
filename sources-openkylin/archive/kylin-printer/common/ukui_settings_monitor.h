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
#ifndef UKUI_SETTINGS_MONITOR_H
#define UKUI_SETTINGS_MONITOR_H

#include <QObject>
#include "singleton.h"

//class GsettingsMonitor;
class HorizontalOrVerticalMode;

class UkuiSettingsMonitor final : public QObject,
                                  public Singleton<UkuiSettingsMonitor>
{
    Q_OBJECT
    friend class Singleton<UkuiSettingsMonitor>;
private:
    UkuiSettingsMonitor();
    ~UkuiSettingsMonitor() override;
public:
    enum TabletStatus {
        None = 0,
        PADHorizontalMode = 1, //平板横屏
        PADVerticalMode = 2,   //平板竖屏
        PCMode = 3             // pc模式
    };

    enum StyleStatus{
        NONE = 0,
        LIGHT,
        DARK
    };


    void startTest(TabletStatus status = None);
//    int transparency() const;
//    StyleStatus styleStatus() const;
    TabletStatus tabletMode() const;

signals:
//    void transparencyUpdate(int transparency);
//    void styleStatusUpdate(StyleStatus styleStatus);
    void tabletModeUpdate(TabletStatus tabletMode);

private:
//    GsettingsMonitor *d_ptrGsetting{nullptr};
    HorizontalOrVerticalMode *d_ptrDbus{nullptr};
};




#endif // UKUI_SETTINGS_MONITOR_H