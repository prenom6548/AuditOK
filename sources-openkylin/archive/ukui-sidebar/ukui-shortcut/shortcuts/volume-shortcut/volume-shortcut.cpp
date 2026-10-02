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
#include "volume-shortcut.h"
#include <QDebug>
#include <QtMath>
#include "ukui-quick/platform/dbus-connector.h"

#define AUDIO_VOLUME_HIGH_SYMBOLIC                "audio-volume-high-24-symbolic"
#define AUDIO_VOLUME_LOW_SYMBOLIC                 "audio-volume-low-24-symbolic"
#define AUDIO_VOLUME_MEDIUM_SYMBOLIC              "audio-volume-medium-24-symbolic"
#define AUDIO_VOLUME_MUTED_SYMBOLIC               "audio-volume-muted-24-symbolic"

#define UKUI_VOLUME_CONTROL_NAME                  "org.ukui.volume.control"
#define UKUI_VOLUME_CONTROL_PATH                  "/org/ukui/volume/control"
#define UKUI_VOLUME_CONTROL_INTERFACE             "org.ukui.volume.control"
#define UKUI_VOLUME_BROOST_INTERFACE              "org.ukui.audio.settings"

using namespace UkuiShortcut;
using namespace UkuiQuick;

VolumeShortcut::VolumeShortcut(QObject *parent) : Shortcut(parent)
{
    initMetaData();

    m_currentStatusInfo.setName(tr("Audio"));
    m_currentStatusInfo.setValue(0);
    m_currentStatusInfo.setIcon(AUDIO_VOLUME_MUTED_SYMBOLIC);
    m_currentStatusInfo.setToolTip(tr("Volume"));
    m_currentStatusInfo.setWidgetId("org.ukui.shortcut.audio");
    m_currentStatusInfo.setIconClickable(true);

    DbusConnector* dbusConnector = new DbusConnector(UKUI_VOLUME_CONTROL_NAME,
                                                     UKUI_VOLUME_CONTROL_PATH,
                                                     UKUI_VOLUME_CONTROL_INTERFACE,
                                                     QDBusConnection::sessionBus());
    connect(dbusConnector, &DbusConnector::ready, this, &VolumeShortcut::onDbusConnectorReady);
    dbusConnector->start();
}

void VolumeShortcut::initDBusConnect()
{
    if (!QDBusConnection::sessionBus().connect(UKUI_VOLUME_CONTROL_NAME,
                                               UKUI_VOLUME_CONTROL_PATH,
                                               UKUI_VOLUME_CONTROL_INTERFACE,
                                               "volumeChanged", this, SLOT(onVolumeChanged(qint32, qint32, QDBusVariant)))) {
        qDebug() << "VolumeShortcut: QDBusConnection::sessionBus() connect volumeChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_VOLUME_CONTROL_NAME,
                                               UKUI_VOLUME_CONTROL_PATH,
                                               UKUI_VOLUME_CONTROL_INTERFACE,
                                               "muteChanged", this, SLOT(onMuteChanged(qint32, qint32, bool)))) {
        qDebug() << "VolumeShortcut: QDBusConnection::sessionBus() connect muteChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_VOLUME_CONTROL_NAME,
                                               UKUI_VOLUME_CONTROL_PATH,
                                               UKUI_VOLUME_CONTROL_INTERFACE,
                                               "deviceAdjust", this, SLOT(onDeviceAdjust(qint32)))) {
        qDebug() << "VolumeShortcut: QDBusConnection::sessionBus() connect deviceAdjust failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_VOLUME_CONTROL_NAME,
                                               UKUI_VOLUME_CONTROL_PATH,
                                               UKUI_VOLUME_CONTROL_INTERFACE,
                                               "deviceChanged", this, SLOT(onDeviceChanged(qint32, QString, QString)))) {
        qDebug() << "VolumeShortcut: QDBusConnection::sessionBus() connect deviceChanged failed";
    }

    if (!QDBusConnection::sessionBus().connect(UKUI_VOLUME_CONTROL_NAME,
                                               UKUI_VOLUME_CONTROL_PATH,
                                               UKUI_VOLUME_BROOST_INTERFACE,
                                               "changed", this, SLOT(onChanged(QString, QDBusVariant)))) {
        qDebug() << "VolumeShortcut: QDBusConnection::sessionBus() connect changed failed";
    }
}

VolumeShortcut::~VolumeShortcut()
{
    if (m_volumeDeviceInterface) {
        delete m_volumeDeviceInterface;
        m_volumeDeviceInterface = nullptr;
    }
    if (m_volumeBoostInterface) {
        delete m_volumeBoostInterface;
        m_volumeBoostInterface = nullptr;
    }
}

void VolumeShortcut::active(PluginMetaType::Action action)
{
    switch (action) {
    case PluginMetaType::Click: {
        if (!m_volumeAvailable || !m_isEnable) return;
        QDBusPendingReply<void> getReply = m_volumeDeviceInterface->asyncCall(QStringLiteral("setMute"), 1, -1, !m_mute);
        watchSetVolumeArgs(getReply, QStringLiteral("setMute"));
        break;
    }
    case PluginMetaType::LongClick:
        break;
    case PluginMetaType::MenuRequest:
        break;
    default:
        break;
    }
}

void VolumeShortcut::setValue(int value)
{
    if (value == m_currentVolume) return;
    if (m_volumeDeviceInterface && m_isEnable && m_volumeAvailable) {
        if(m_mute && value > 0) {
            QDBusPendingReply<void> getReply = m_volumeDeviceInterface->asyncCall(QStringLiteral("setMute"), 1, -1, !m_mute);
            watchSetVolumeArgs(getReply, QStringLiteral("setMute"));
        }
        QDBusPendingReply<void> getReply = m_volumeDeviceInterface->asyncCall(QStringLiteral("setVolume"), 1, -1, value);
        watchSetVolumeArgs(getReply, QStringLiteral("setVolume"));
    }
}

const StatusInfo VolumeShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool VolumeShortcut::isEnable()
{
    return m_isEnable;
}

void VolumeShortcut::onDeviceAdjust(qint32 type)
{
    getDeviceVolume();
}

void VolumeShortcut::onDeviceChanged(qint32 type, QString portName, QString cardName)
{
    getDeviceVolume();
}

void VolumeShortcut::getDeviceVolume()
{
    QDBusPendingReply<bool> getReply = m_volumeDeviceInterface->asyncCall(QStringLiteral("isValidDevice"), 1);
    auto watcher = new QDBusPendingCallWatcher(getReply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, getReply] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "VolumeShortcut: asyncCall "<< "isValidDevice" << " failed !!!" << self->error().message();
            } else {
                if (getReply.isValid()) {
                    m_volumeAvailable = getReply.value();
                    if (!m_volumeAvailable) {
                        m_currentStatusInfo.setValue(0);
                        m_currentStatusInfo.setIcon(AUDIO_VOLUME_MUTED_SYMBOLIC);
                        m_currentStatusInfo.setDisable(true);
                        Q_EMIT statusChanged(m_currentStatusInfo);
                    } else {
                        initVolumeArgs<qint32>(QStringLiteral("getVolume"), CurrentVolume);
                        initVolumeArgs<bool>(QStringLiteral("getMute"), Mute);
                    }
                } else {
                    qDebug() << "VolumeShortcut: api " << "isValidDevice" << " return value is not valid" <<getReply.error();
                }
            }
            self->deleteLater();
        }
    });
}

void VolumeShortcut::onChanged(QString key, QDBusVariant value)
{
    if (key == "volume-increase") {
        m_isIncrease = value.variant().toBool();
    } else if (key == "volume-increase-value") {
        m_volumeMaxValue = value.variant().toInt();
    }
    m_currentStatusInfo.setMaxValue(m_isIncrease ? m_volumeMaxValue : 100);
    Q_EMIT statusChanged(m_currentStatusInfo);
    if (!m_volumeAvailable || !m_isEnable) return;
    changeValue(m_currentVolume);
}

void VolumeShortcut::onDbusConnectorReady(QDBusInterface *interface)
{
    if (!interface || !interface->isValid()) return;
    m_volumeDeviceInterface = interface;
    m_volumeBoostInterface = new QDBusInterface(UKUI_VOLUME_CONTROL_NAME, UKUI_VOLUME_CONTROL_PATH,
                                                UKUI_VOLUME_BROOST_INTERFACE, QDBusConnection::sessionBus());
    initVolumeArgs<bool>(QStringLiteral("getMute"), Mute);
    initVolumeArgs<bool>(QStringLiteral("getVolumeBoostStatus"), IsIncrease);
    initVolumeArgs<qint32>(QStringLiteral("getVolumeBoostValue"), VolumeMaxValue);
    initVolumeArgs<qint32>(QStringLiteral("getVolume"), CurrentVolume);
    onDeviceAdjust();
    initDBusConnect();
}

void VolumeShortcut::onVolumeChanged(qint32 type, qint32 index, QDBusVariant value)
{
    if (type == 1) {
        m_currentVolume = value.variant().toDouble();
        if (!m_volumeAvailable || !m_isEnable) return;
        changeValue(m_currentVolume);
    }
}

void VolumeShortcut::onMuteChanged(qint32 type, qint32 index, bool isMute)
{
    if (type == 1) {
        m_mute = isMute;
        if (!m_volumeAvailable || !m_isEnable) return;
        mute(m_mute);
    }
}

void VolumeShortcut::changeValue(double volume)
{
    if (volume < 0 || volume > m_volumeMaxValue) {
        qWarning() << "VolumeShortcut value error: " << volume;
        volume = volume < 0 ? 0 : m_volumeMaxValue;
    }
    if (volume == 0 || m_mute) {
        m_currentStatusInfo.setIcon(AUDIO_VOLUME_MUTED_SYMBOLIC);
    } else if (volume <= 33) {
        m_currentStatusInfo.setIcon(AUDIO_VOLUME_LOW_SYMBOLIC);
    } else if (volume <= 66) {
        m_currentStatusInfo.setIcon(AUDIO_VOLUME_MEDIUM_SYMBOLIC);
    } else {
        m_currentStatusInfo.setIcon(AUDIO_VOLUME_HIGH_SYMBOLIC);
    }
    m_currentStatusInfo.setDisable(false);
    m_currentStatusInfo.setValue(volume);
    Q_EMIT statusChanged(m_currentStatusInfo);
}

QMap<PluginMetaType::SystemMode, PluginMetaData> VolumeShortcut::pluginMetaData()
{
    return m_metaData;
}

void VolumeShortcut::initMetaData()
{
    PluginMetaData pc {true, 1, PluginMetaType::PluginType::ProgressBar};
    PluginMetaData tablet {true, 1, PluginMetaType::PluginType::ProgressBar};
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void VolumeShortcut::mute(bool mute)
{
    m_mute = mute;
    changeValue(m_currentStatusInfo.getValue());
}

VolumeShortcutPlugin::VolumeShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList VolumeShortcutPlugin::translations()
{
    return {QStringLiteral("volume-shortcut")};
}

Shortcut *VolumeShortcutPlugin::createShortcut()
{
    return new VolumeShortcut;
}

template<typename T>
void VolumeShortcut::initVolumeArgs(const QString &apiName, const int &argType)
{
    QDBusPendingReply<T> getReply;
    if (argType == IsIncrease || argType == VolumeMaxValue) {
        getReply = m_volumeBoostInterface->asyncCall(apiName);
    } else if (argType == CurrentVolume || argType == Mute) {
        getReply = m_volumeDeviceInterface->asyncCall(apiName, 1, -1);
    }
    auto watcher = new QDBusPendingCallWatcher(getReply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, getReply, apiName, argType] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "VolumeShortcut: asyncCall"<< apiName << "failed !!!" << self->error().message();
            } else {
                bool isEnable = true;
                if (getReply.isValid()) {
                    switch (argType) {
                    case CurrentVolume:
                        m_currentVolume = getReply.value();
                        break;
                    case IsIncrease:
                        m_isIncrease = getReply.value();
                        break;
                    case VolumeMaxValue:
                        m_volumeMaxValue = getReply.value();
                        m_currentStatusInfo.setMaxValue(m_isIncrease ? m_volumeMaxValue : 100);
                        Q_EMIT statusChanged(m_currentStatusInfo);
                        break;
                    case Mute:
                        m_mute = getReply.value();
                        mute(m_mute);
                        break;
                    }
                    changeValue(m_currentVolume);
                } else {
                    qDebug() << "VolumeShortcut: api"<< apiName << "return value is not valid" <<getReply.error();
                    if (argType == CurrentVolume) {
                        isEnable = false;
                    }
                }

                if (m_isEnable != isEnable) {
                    m_isEnable = isEnable;
                    Q_EMIT enableStatusChanged(m_isEnable);
                }
            }
            self->deleteLater();
        }
    });
}

void VolumeShortcut::watchSetVolumeArgs(const QDBusPendingReply<void> &getReply, const QString &apiName)
{
    auto watcher = new QDBusPendingCallWatcher(getReply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, apiName] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug() << "VolumeShortcut: asyncCall" << apiName << "failed !!!" << self->error().message();
            }
            self->deleteLater();
        }
    });
}
