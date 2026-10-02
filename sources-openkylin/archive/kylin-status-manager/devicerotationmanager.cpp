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

#include "devicerotationmanager.h"
#include "ConfigurationManager/configurationmanager.h"

#include <unordered_map>

DeviceRotationManager::DeviceRotationManager(QObject *parent)
    :QObject(parent)
    ,m_autoRotateEnable(false)
    , m_orientation("normal")
    , m_configuration(ConfigurationManager::GetInstance())
    , m_orientationSensor(new QOrientationSensor(this))
{
    connectAutoRotateSignal();
    initOrientationSensor();
}

DeviceRotationManager::~DeviceRotationManager()
{

}

QString DeviceRotationManager::enumConvertToString(
        QOrientationReading::Orientation orientation)
{
    std::unordered_map<QOrientationReading::Orientation,QString>orientationValue = {
        {QOrientationReading::Orientation::Undefined, "undefined"},
        {QOrientationReading::Orientation::TopUp, "normal"},
        {QOrientationReading::Orientation::TopDown , "upside-down"},
        {QOrientationReading::Orientation::LeftUp, "left"},
        {QOrientationReading::Orientation::RightUp, "right"},
        {QOrientationReading::Orientation::FaceUp, "unknown"},
        {QOrientationReading::Orientation::FaceDown, "unknown"},
    };
    if (orientationValue.find(orientation) != orientationValue.end()) {
        return orientationValue[orientation];
    }
    return "undefined";
}

QOrientationReading::Orientation DeviceRotationManager::getSensorOrientation() const
{
    if (!isOrientationSensorAvailable()) {
        return QOrientationReading::Orientation::Undefined;
    }

    return m_orientationSensor->reading()->orientation();
}

bool DeviceRotationManager::isOrientationSensorAvailable() const
{
    return m_orientationSensorAvailable;
}

void DeviceRotationManager::connectAutoRotateSignal()
{
    connect(this, &DeviceRotationManager::autoRotateEnabledChange, this, &DeviceRotationManager::onDeviceOrientationChanged);
}

void DeviceRotationManager::initOrientationSensor()
{
    connect(m_orientationSensor, &QOrientationSensor::readingChanged, this, &DeviceRotationManager::onDeviceOrientationChanged);
    m_orientationSensorAvailable = m_orientationSensor->start();
}

void DeviceRotationManager::onDeviceOrientationChanged()
{
    if (isAutoRotateEnabled()) {
        setDeviceOrientationPermanent(enumConvertToString(getSensorOrientation()));
    }
}

bool DeviceRotationManager::setDeviceOrientation(QString orientation)
{
    if (m_orientation == orientation) {
        return true;
    }

    m_orientation = orientation;
    emit orientationChanged(m_orientation);
    return true;
}

bool DeviceRotationManager::setDeviceOrientationPermanent(QString orientation)
{
    setDeviceOrientation(orientation);
    m_configuration->setDeviceOrientation(orientation);
    return true;
}

bool DeviceRotationManager::isAutoRotateEnabled() const
{
    return m_autoRotateEnable;
}

QString DeviceRotationManager::getDeviceOrientation() const
{
    return m_orientation;
}

bool DeviceRotationManager::setAutoRotateEnabled(bool autoRotateEnable)
{
    if (m_autoRotateEnable == autoRotateEnable) {
        return true;
    }

    m_autoRotateEnable = autoRotateEnable;
    emit autoRotateEnabledChange(m_autoRotateEnable);
    return true;
}
