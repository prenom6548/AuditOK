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
#ifndef VOLUMESHORTCUT_H
#define VOLUMESHORTCUT_H
#include "ukui-shortcut-plugin.h"
#include <QtDBus>

namespace UkuiShortcut {

class VolumeShortcutPlugin : public UkuiShortcutPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID UKUI_SHORTCUT_PLUGIN_IFACE_IID FILE "volume-shortcut.json")
    Q_INTERFACES(UkuiShortcut::UkuiShortcutPlugin)
public:
    explicit VolumeShortcutPlugin(QObject *parent = nullptr);

    QString pluginId() override { return QStringLiteral("VolumeShortcut"); };
    QStringList translations() override;
    Shortcut *createShortcut() override;
};

class  VolumeShortcut : public Shortcut
{
    Q_OBJECT
public:
    enum ArgsType {
        CurrentVolume = 0,
        IsIncrease,
        VolumeMaxValue,
        Mute
    };
    Q_ENUM(ArgsType)
    explicit VolumeShortcut(QObject *parent = nullptr);
    ~VolumeShortcut() override;

    QString pluginId() override {return QStringLiteral("VolumeShortcut");}
    QMap<PluginMetaType::SystemMode, PluginMetaData> pluginMetaData() override;
    void active(PluginMetaType::Action action) override;
    void setValue(int value) override;
    const StatusInfo currentStatus() override;
    bool isEnable() override;
private Q_SLOTS:
    void onDeviceAdjust(qint32 type = 1);
    void onDeviceChanged(qint32 type, QString portName, QString cardName);
    void onDbusConnectorReady(QDBusInterface* interface);
    void onVolumeChanged(qint32 type, qint32 index, QDBusVariant value);
    void onMuteChanged(qint32 type, qint32 index, bool isMute);
    void onChanged(QString key, QDBusVariant value);

private:
    void changeValue(double volume);
    void mute(bool mute);
    inline void initMetaData();
    void watchSetVolumeArgs(const QDBusPendingReply<void> &getReply, const QString &apiName);
    void initDBusConnect();
    template <typename T>
    void initVolumeArgs(const QString &apiName,const int &argType);
    void getDeviceVolume();

    bool m_isIncrease = false;
    int  m_volumeMaxValue = 100;
    double m_currentVolume = 0;

    bool m_isEnable = false;
    bool m_mute = false;
    StatusInfo m_currentStatusInfo;
    QMap<PluginMetaType::SystemMode, PluginMetaData> m_metaData;
    QDBusInterface *m_volumeDeviceInterface = nullptr;
    QDBusInterface *m_volumeBoostInterface = nullptr;
    bool m_volumeAvailable = false;
};
}
#endif // VOLUMESHORTCUT_H
