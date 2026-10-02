/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 */
#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QObject>
#include <QGSettings>

namespace UkuiWindowSwitch {

class GlobalSettings : public QObject
{
    Q_OBJECT

public:
    static GlobalSettings *globalInstance(QObject *parent = nullptr);
    ~GlobalSettings() = default;
    // 任务栏位置
    int panelPosition();

private:
    explicit GlobalSettings(QObject *parent = nullptr);
    void initPanelGSettings();

    QGSettings *m_panelGSettings {nullptr};
    int m_panelPosition = 0;

Q_SIGNALS:
    void panelPositionChanged(int);
};
} // UkuiWindowSwitch
#endif // GLOBALSETTINGS_H
