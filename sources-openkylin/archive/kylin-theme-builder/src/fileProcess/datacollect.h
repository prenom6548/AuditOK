/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef DATACOLLECT_H
#define DATACOLLECT_H

#include <QObject>

class DataCollect : public QObject
{
    Q_OBJECT
public:
    explicit DataCollect(QObject *parent = nullptr);
    enum WindowButton {
        m_GlobalTheme = 0x0,
        m_IconTheme = 0x1,
        m_CursorTheme = 0x2,
        m_PlymouthTheme = 0x3,
        m_GrubTheme = 0x4
    };
    enum NavigationButton {
        n_GlobalTheme = 0x0,
        n_IconTheme = 0x1,
        n_CursorTheme = 0x2,
        n_PlymouthTheme = 0x3,
        n_GrubTheme = 0x4
    };
    enum GlobalThemeChangeWidget {
        g_CoverWidget = 0x0,
        g_WallPaperWidget= 0x1,
        g_ExteriorWidget = 0x2,
        g_FilletWidget = 0x3,
        g_AccentColorWidget = 0x4,
        g_TransparencyWidget = 0x5,
        g_WindowFilletWidget = 0x6
    };
    static void sendMainWindowKdkDataAsync(WindowButton theme);
    static void sendNavigationKdkDataAsync(NavigationButton theme);
    static void sendGlobalThemeKdkDataAsync(GlobalThemeChangeWidget widget);
signals:

};

#endif // DATACOLLECT_H
