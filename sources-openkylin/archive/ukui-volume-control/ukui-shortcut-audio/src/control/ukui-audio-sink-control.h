/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
* Authors: zhouyong <zhouyong01@kylinos.cn>
*
*/
#ifndef UKUIAUDIOSINKCONTROL_H
#define UKUIAUDIOSINKCONTROL_H

#include <QObject>
#include <QDebug>
#include <QSet>

#include "DBusClient.h"
#include "../extra/BaseType.h"

class UKUIAudioSinkControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sinkVolumeValue READ sinkVolumeValue WRITE setSinkVolumeValue NOTIFY sinkVolumeValueChanged)
    Q_PROPERTY(int sinkMuteStatus READ sinkMuteStatus WRITE setSinkMuteStatus NOTIFY sinkMuteStatusChanged)
    Q_PROPERTY(QString sinkVolumeStatusIcon READ sinkVolumeStatusIcon WRITE setSinkVolumeStatusIcon NOTIFY sinkVolumeStatusIconChanged)
    Q_PROPERTY(bool sinkVolumeBoostStatus READ sinkVolumeBoostStatus WRITE setSinkVolumeBoostStatus NOTIFY sinkVolumeBoostStatusChanged)
    Q_PROPERTY(int sinkVolumeBoostValue READ sinkVolumeBoostValue WRITE setSinkVolumeBoostValue NOTIFY sinkVolumeBoostValueChanged)

public:
    static UKUIAudioSinkControl& getInstance();

    bool initData();
    bool initSlots();
    bool initView();

    int sinkVolumeValue() const { return m_sinkVolumeValue;}
    void setSinkVolumeValue(int sinkVolumeValue) {
        qDebug() << __func__ << "sinkVolumeValue:" << sinkVolumeValue << "m_sinkVolumeValue:" << m_sinkVolumeValue;
        //start no sink device
        if (getEnabledSinkDeviceCount() <= 0) {
            sinkVolumeValue = 0;
        }
        //end
        m_sinkVolumeValue = sinkVolumeValue;
        emit sinkVolumeValueChanged(m_sinkVolumeValue);

        QString sinkVolumeStatusIcon;
        if (sinkMuteStatus() || (sinkVolumeValue == 0)) {
            sinkVolumeStatusIcon = "audio-volume-muted-symbolic";
        } else if (sinkVolumeValue > 66) {
            sinkVolumeStatusIcon = "audio-volume-high-symbolic";
        } else if (sinkVolumeValue > 33) {
            sinkVolumeStatusIcon = "audio-volume-medium-symbolic";
        } else {
            sinkVolumeStatusIcon = "audio-volume-low-symbolic";
        }
        setSinkVolumeStatusIcon(sinkVolumeStatusIcon);
    }

    int sinkMuteStatus() const { return m_sinkMuteStatus;}
    void setSinkMuteStatus(bool sinkMuteStatus) {
        qDebug() << __func__ << "sinkMuteStatus:" << sinkMuteStatus << "m_sinkMuteStatus:" << m_sinkMuteStatus;
//        if (sinkMuteStatus != m_sinkMuteStatus) {
            m_sinkMuteStatus = sinkMuteStatus;
            emit sinkMuteStatusChanged(m_sinkMuteStatus);

            QString sinkVolumeStatusIcon;
            if (sinkMuteStatus || (sinkVolumeValue() == 0)) {
                sinkVolumeStatusIcon = "audio-volume-muted-symbolic";
            } else if (sinkVolumeValue() > 66) {
                sinkVolumeStatusIcon = "audio-volume-high-symbolic";
            } else if (sinkVolumeValue() > 33) {
                sinkVolumeStatusIcon = "audio-volume-medium-symbolic";
            } else {
                sinkVolumeStatusIcon = "audio-volume-low-symbolic";
            }
            setSinkVolumeStatusIcon(sinkVolumeStatusIcon);
//        }
    }

    QString sinkVolumeStatusIcon() const { return m_sinkVolumeStatusIcon;}
    void setSinkVolumeStatusIcon(QString sinkVolumeStatusIcon) {
        qDebug() << __func__ << "sinkVolumeStatusIcon:" << sinkVolumeStatusIcon << "m_sinkVolumeStatusIcon:" << m_sinkVolumeStatusIcon;
        // if (sinkVolumeStatusIcon != m_sinkVolumeStatusIcon) {
            m_sinkVolumeStatusIcon = sinkVolumeStatusIcon;
            emit sinkVolumeStatusIconChanged(m_sinkVolumeStatusIcon);
        // }
    }

    bool sinkVolumeBoostStatus() const {return m_sinkVolumeBoostStatus;}
    void setSinkVolumeBoostStatus(bool sinkVolumeBoostStatus) {
        qDebug() << __func__ << "sinkVolumeBoostStatus:" << sinkVolumeBoostStatus << "m_sinkVolumeBoostStatus:" << m_sinkVolumeBoostStatus;
        // if (sinkVolumeBoostStatus != m_sinkVolumeBoostStatus) {
            m_sinkVolumeBoostStatus = sinkVolumeBoostStatus;
            emit sinkVolumeBoostStatusChanged(m_sinkVolumeBoostStatus);
        // }
    }

    int sinkVolumeBoostValue() const { return m_sinkVolumeBoostValue;}
    void setSinkVolumeBoostValue(int sinkVolumeBoostValue) {
        qDebug() << __func__ << "sinkVolumeBoostValue:" << sinkVolumeBoostValue << "m_sinkVolumeBoostValue:" << m_sinkVolumeBoostValue;
        // if (sinkVolumeBoostValue != m_sinkVolumeBoostValue) {
            m_sinkVolumeBoostValue = sinkVolumeBoostValue;
            emit sinkVolumeBoostValueChanged(m_sinkVolumeBoostValue);
        // }
    }

signals:
    void sinkVolumeValueChanged(int sinkVolumeValue);
    void sinkMuteStatusChanged(bool sinkMuteStatus);
    void sinkVolumeStatusIconChanged(QString sinkVolumeStatusIcon);
    void sinkVolumeBoostStatusChanged(bool sinkVolumeBoostStatus);
    void sinkVolumeBoostValueChanged(int sinkVolumeBoostValue);

signals:
    void sinkDeviceChanged(QString, QString);
    void sinkDeviceAdjust();

private Q_SLOTS:
    void volumeChangedSlots(int, int, const QDBusVariant&);
    void muteChangedSlots(int, int, bool);
    void deviceChangedSlots(int, const QString&, const QString&);
    void deviceAdjustSlots(int);
    void addStreamSlots(int, int, int, bool, const QString&, const QString&);
    void removeStreamSlots(int, int);
    void settingsChangedSlots(const QString&, const QDBusVariant&);
    void streamVolumeChangedSlots(QString, int, bool);

public:
    bool getSinkVolume(int, QVariant& retValue);
    Q_INVOKABLE void updateSinkVolume(QVariant retValue);
    bool setSinkVolume(int, int);

    bool getSinkMute(int, QVariant& retValue);
    Q_INVOKABLE void updateSinkMute(QVariant retValue);
    bool setSinkMute(int, bool);

    bool getSinkAvailablePortList(QVariant& retValue);
    Q_INVOKABLE void updateSinkAvailablePortList(QVariant retValue);

    bool getSinkInputList(QVariant& retValue);
    Q_INVOKABLE void updateSinkInputList(QVariant retValue);

    Q_INVOKABLE void updateSinkInputVolume(QVariant retValue);

    bool setSinkInputVolume(int, int);

    bool sendSinkInputVolumeChanged(const QString&, int, bool);

    bool getSinkDefaultDevice(int, QVariant& retValue);
    Q_INVOKABLE void updateSinkDefaultDevice(QVariant retValue);
    Q_INVOKABLE void updateSinkDefaultDevice(const QString&, const QString&);
    bool setSinkDefaultDevice(int, const QString&, const QString&);

    bool getSinkVolumeBoostStatus(QVariant& retValue);
    Q_INVOKABLE void updateSinkVolumeBoostStatus(QVariant retValue);

    bool getSinkVolumeBoostValue(QVariant& retValue);
    Q_INVOKABLE void updateSinkVolumeBoostValue(QVariant retValue);

    bool isExitMultiAudioDevice();
    bool getSinkMultiAudioCombineStatus() const;
    QList<ClientDefaultDeviceInfo> getSinkList() const;

    Q_INVOKABLE int getSinkDeviceCount();
    Q_INVOKABLE int getEnabledSinkDeviceCount();

    Q_INVOKABLE void releaseData();
    Q_INVOKABLE void releaseSlots();

private:
    UKUIAudioSinkControl() = default;
    UKUIAudioSinkControl(const UKUIAudioSinkControl&) = delete;
    UKUIAudioSinkControl(UKUIAudioSinkControl&&) = delete;
    UKUIAudioSinkControl operator=(const UKUIAudioSinkControl&) = delete;
    UKUIAudioSinkControl operator=(UKUIAudioSinkControl&&) = delete;
    ~UKUIAudioSinkControl();

    void collectKylinRecorderInputIndices(const ClientStreamMediaInfo& info);
    void applyKylinRecorderVolumeEnabled();

private:
    int m_sinkVolumeValue = -1;
    bool m_sinkMuteStatus = false;
    QString m_sinkVolumeStatusIcon = "";
    bool m_sinkVolumeBoostStatus = false;
    int m_sinkVolumeBoostValue = -1;

    bool m_isExitMultiAudioDevice = false;
    ClientDefaultDeviceInfo m_SinkMutiAudioDeviceInfo;

    QSet<int> m_kylinRecorderInputIndices;

};

#endif // UKUIAUDIOSINKCONTROL_H
