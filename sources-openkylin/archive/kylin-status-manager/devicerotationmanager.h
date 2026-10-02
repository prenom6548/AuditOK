/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef _DEVICE_ROTATION_MANAGER_H
#define _DEVICE_ROTATION_MANAGER_H

#include <QObject>
#include <QOrientationSensor>
#include "KmreAppManager/kmrenotificationwindowmanager.h"
class QOrientationSensor;
class ConfigurationManager;
class DeviceRotationManager : public QObject {
    Q_OBJECT

public:
    explicit DeviceRotationManager(QObject *parent = nullptr);
    ~DeviceRotationManager() override;

    bool isOrientationSensorAvailable() const;
    bool isAutoRotateEnabled() const;
    QString getDeviceOrientation() const;

public slots:
    bool setAutoRotateEnabled(bool autoRotateEnable);
    void onDeviceOrientationChanged();
    bool setDeviceOrientation(QString orientation);
    bool setDeviceOrientationPermanent(QString orientation);

signals:
    void autoRotateEnabledChange(bool enabled);
    void orientationChanged(QString orientation);

private:
    void connectAutoRotateSignal();
    void initOrientationSensor();
    QString enumConvertToString(QOrientationReading::Orientation orientation);
    QOrientationReading::Orientation getSensorOrientation() const;

private:
    bool m_orientationSensorAvailable;
    bool m_autoRotateEnable;
    QString m_orientation;
    ConfigurationManager *m_configuration = nullptr;
    QOrientationSensor *m_orientationSensor = nullptr;

};

#endif
