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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef BRIGHTNESSSHORTCUT_H
#define BRIGHTNESSSHORTCUT_H

class QDBusInterface;
#include "ukui-shortcut-plugin.h"

namespace UkuiShortcut {

class BrightnessShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "brightness-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit BrightnessShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("BrightnessShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class  BrightnessShortcut : public Shortcut
{
    Q_OBJECT
public:
    explicit BrightnessShortcut(QObject *parent = nullptr);
    ~BrightnessShortcut() override;

    QString pluginId() override;
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    void setValue(int value) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;

private:
    inline void initMetaData();
    inline void updateStatusInfo(quint32 brightness);
    void setMaxValue();

private Q_SLOTS:
    void onDbusConnectorReady(QDBusInterface* interface);
    void changeValue(quint32 brightness);
    void onEnableChanged(bool enable);
    void onEnhanceChanged(bool enhance);

private:
    bool m_isEnable = false;
    bool m_enhance = false;
    double m_currentBrightness = 0;
    int m_enhancedBrightness = 0;
    QDBusInterface *m_interface = nullptr;
    StatusInfo m_currentStatusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
};
}

#endif // BRIGHTNESSSHORTCUT_H
