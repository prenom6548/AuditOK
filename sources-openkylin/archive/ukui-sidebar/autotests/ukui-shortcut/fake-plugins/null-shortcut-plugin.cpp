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

#include "ukui-shortcut-plugin.h"

using namespace UkuiShortcut;

class NullShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "valid-shortcut-plugin.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)

public:
    QString pluginId() override
    {
        return QStringLiteral("null-shortcut-plugin");
    }

    QStringList translations() override
    {
        return {};
    }

    Shortcut *createShortcut() override
    {
        return nullptr;
    }
};

#include "null-shortcut-plugin.moc"
