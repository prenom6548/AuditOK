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

class FakeShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit FakeShortcut(QObject *parent = nullptr) : Shortcut(parent) {}

    QString pluginId() override
    {
        return QStringLiteral("fake.shortcut");
    }

    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override
    {
        return {};
    }

    void active(PluginMetaType::Action action) override
    {
        Q_UNUSED(action)
    }

    const StatusInfo currentStatus() override
    {
        StatusInfo info;
        info.setName(QStringLiteral("Fake Shortcut"));
        info.setWidgetId(QStringLiteral("fake.widget"));
        return info;
    }
};

class FakeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "valid-shortcut-plugin.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)

public:
    QString pluginId() override
    {
        return QStringLiteral("fake-shortcut-plugin");
    }

    QStringList translations() override
    {
        return {QStringLiteral("missing-shortcut-translation")};
    }

    Shortcut *createShortcut() override
    {
        return new FakeShortcut;
    }
};

#include "fake-shortcut-plugin.moc"
