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

#ifndef AUTOTESTS_WINDOWS_STUB_SHORT_CUT_MANAGER_H
#define AUTOTESTS_WINDOWS_STUB_SHORT_CUT_MANAGER_H

#include <QMap>
#include <QPair>
#include <QString>

#include "value-type.h"

namespace UkuiShortcut {

class Shortcut
{
};

class StatusInfo
{
    Q_GADGET
public:
    enum MenuItem {
        InvalidMenuItem = 0,
        DefaultMenuItem
    };
    Q_ENUM(MenuItem)
};

class PluginMetaType
{
    Q_GADGET
public:
    enum Action {
        InvalidAction = 0,
        OpenAction
    };
    Q_ENUM(Action)

    enum PluginType {
        InvalidPlugin = 0,
        SystemPlugin
    };
    Q_ENUM(PluginType)

    enum SystemMode {
        InvalidMode = 0,
        DesktopMode
    };
    Q_ENUM(SystemMode)

    enum PredefinedAction {
        NoAction = 0,
        Hide
    };
    Q_ENUM(PredefinedAction)
};

class PluginStateType
{
    Q_GADGET
public:
    enum AddState {
        InvalidState = 0,
        Added
    };
    Q_ENUM(AddState)
};

class ShortcutManager
{
public:
    static ShortcutManager *getInstance();

    QList<Shortcut *> getShortcuts();
    QPair<bool, QString> getWidgetInfo(const QString &widgetId);

    void setWidgetInfo(const QString &widgetId, const QString &returnButtonName);
    void reset();

private:
    QMap<QString, QString> m_widgetInfo;
};

} // namespace UkuiShortcut

#endif
