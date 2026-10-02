/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef PLUGINSTATE_H
#define PLUGINSTATE_H

#include <QObject>

namespace UkuiShortcut {

class PluginStateType
{
    Q_GADGET
public:
    enum AddState {
        Add = 0, //已添加
        NotAdd,  //未添加
    };
    Q_DECLARE_FLAGS(AddStates, AddState)
    Q_ENUM(AddState)

    enum FixState {
        Fix = 0, //已固定
        NotFix,  //未固定
    };
    Q_DECLARE_FLAGS(FixStates, FixState)
    Q_ENUM(FixState)
};
}
#endif // PLUGINSTATE_H
