/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

//
// Created by hxf on 22-7-21.
//

#ifndef UKUI_SIDEBAR_TEST_SCROLLER_BAR_H
#define UKUI_SIDEBAR_TEST_SCROLLER_BAR_H

#include "ukui-shortcut-plugin.h"

namespace UkuiShortcut {

class TestScrollerBar : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UkuiShortcutPluginIface_iid FILE "power-mode-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)

public:
    explicit TestScrollerBar(QObject *parent = nullptr);

    ~TestScrollerBar() override;

    PluginType pluginType() override;

    void active(Action action) override;

    void setValue(int value) override;

    StatusInfo currentStatus() override;

    bool isEnable() override;

private:
    void updateSize(double size);

private:
    int m_size = 0;
    StatusInfo m_currentStatus;
};

} // UkuiShortCut

#endif //UKUI_SIDEBAR_TEST_SCROLLER_BAR_H
