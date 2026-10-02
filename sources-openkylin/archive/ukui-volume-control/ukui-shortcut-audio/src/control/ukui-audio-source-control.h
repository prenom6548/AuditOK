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
#ifndef UKUIAUDIOSOURCECONTROL_H
#define UKUIAUDIOSOURCECONTROL_H

#include <QObject>
#include <QDebug>

#include "DBusClient.h"

class UKUIAudioSourceControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int sourceVolumeValue READ sourceVolumeValue WRITE setSourceVolumeValue NOTIFY sourceVolumeValueChanged)


public:
    static UKUIAudioSourceControl& getInstance();

    bool initData();
    bool initSlots();

    int sourceVolumeValue() const { return m_sourceVolumeValue;}
    void setSourceVolumeValue(int sourceVolumeValue) {
        qDebug() << __func__ << "sourceVolumeValue:" << sourceVolumeValue << "m_sourceVolumeValue:" << m_sourceVolumeValue;
        if (sourceVolumeValue != m_sourceVolumeValue) {
            m_sourceVolumeValue = sourceVolumeValue;
            emit sourceVolumeValueChanged(m_sourceVolumeValue);
        }
    }

signals:
    void sourceVolumeValueChanged(int sourceVolumeValue);

signals:
    void sourceDeviceChanged(QString, QString);
    void sourceDeviceAdjust();

private Q_SLOTS:
    void volumeChangedSlots(int, int, const QDBusVariant&);
    void deviceChangedSlots(int, const QString&, const QString&);
    void deviceAdjustSlots(int);

public:
    bool getSourceVolume(int index, QVariant& retValue);
    Q_INVOKABLE void updateSourceVolume(QVariant retValue);
    bool setSourceVolume(int index, int);

    bool getSourceAvailablePortList(QVariant& retValue);
    Q_INVOKABLE void updateSourceAvailablePortList(QVariant retValue);

    bool getSourceOutputList(QVariant& retValue);
    Q_INVOKABLE void updateSourceInputList(QVariant retValue);

    bool getSourceDefaultDevice(int index, QVariant& retValue);
    Q_INVOKABLE void updateSourceDefaultDevice(QVariant retValue);
    Q_INVOKABLE void updateSourceDefaultDevice(const QString&, const QString&);
    bool setSourceDefaultDevice(int index, const QString&, const QString&);

    Q_INVOKABLE void releaseData();
    Q_INVOKABLE void releaseSlots();

private:
    UKUIAudioSourceControl() = default;
    UKUIAudioSourceControl(const UKUIAudioSourceControl&) = delete;
    UKUIAudioSourceControl(UKUIAudioSourceControl&&) = delete;
    UKUIAudioSourceControl operator=(const UKUIAudioSourceControl&) = delete;
    UKUIAudioSourceControl operator=(UKUIAudioSourceControl&&) = delete;
    ~UKUIAudioSourceControl();

private:
    int m_sourceVolumeValue = -1;

};

#endif // UKUIAUDIOSOURCECONTROL_H
