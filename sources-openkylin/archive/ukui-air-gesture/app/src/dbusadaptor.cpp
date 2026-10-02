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
 */

#include "dbusadaptor.h"

#include <QDebug>
#include <QTimer>

DBusAdaptor::DBusAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    if (QGSettings::isSchemaInstalled(AirGestureSettings::kGestureSchema)) {
        m_pGestureSettings = new QGSettings(AirGestureSettings::kGestureSchema, "", parent);
    }
}

bool DBusAdaptor::isTrialActive() const
{
    return m_trialActive;
}

bool DBusAdaptor::isGestureEnabled(AirGestureType type) const
{
    const char *key = AirGestureSettings::enabledKeyFor(type);
    if (key == nullptr || m_pGestureSettings == nullptr) {
        return false;
    }

    return m_pGestureSettings->get(key).toBool();
}

bool DBusAdaptor::isGestureAllowed(AirGestureType type) const
{
    if (m_trialActive) {
        return m_trialGestureType == type;
    }

    return isGestureEnabled(type);
}

bool DBusAdaptor::isShowIconAllowed(const QString &palmDirection) const
{
    if (m_trialActive) {
        switch (m_trialGestureType) {
        case AirGestureType::Wave_Vertical:
            return palmDirection == QStringLiteral("up") || palmDirection == QStringLiteral("down");
        case AirGestureType::Wave_Horizontal:
            return palmDirection == QStringLiteral("left") || palmDirection == QStringLiteral("right");
        case AirGestureType::Fist:
            return palmDirection == QStringLiteral("up");
        default:
            return false;
        }
    }

    if (palmDirection == QStringLiteral("up")) {
        return isGestureEnabled(AirGestureType::Wave_Vertical)
            || isGestureEnabled(AirGestureType::Fist);
    }
    if (palmDirection == QStringLiteral("down")) {
        return isGestureEnabled(AirGestureType::Wave_Vertical);
    }
    if (palmDirection == QStringLiteral("left") || palmDirection == QStringLiteral("right")) {
        return isGestureEnabled(AirGestureType::Wave_Horizontal);
    }

    return false;
}

void DBusAdaptor::showIcon(const QString &palmDirection)
{
    if (!isShowIconAllowed(palmDirection)) {
        return;
    }

    if (m_trialActive) {
        emit palmDetected(palmDirection);
        return;
    }

    m_mainWindow.setIcon(palmDirection);
    m_mainWindow.show();
}

void DBusAdaptor::hideIcon()
{
    if (m_trialActive) {
        emit palmLost();
    }

    m_mainWindow.hide();
}

void DBusAdaptor::wheel(const QString &direction)
{
    qDebug() << "Wheel triggered:" << direction;

    if (!isGestureAllowed(AirGestureType::Wave_Vertical)) {
        return;
    }

    if (m_trialActive) {
        emit trialGesturePerformed(direction);
        return;
    }

    m_actionExecutor.executeWheel(direction);
}

void DBusAdaptor::horizontal(const QString &direction)
{
    qDebug() << "Horizontal triggered:" << direction;

    if (!isGestureAllowed(AirGestureType::Wave_Horizontal)) {
        return;
    }

    if (m_trialActive) {
        emit trialGesturePerformed(direction);
        return;
    }

    m_actionExecutor.executeHorizontal(direction);
}

void DBusAdaptor::ptrsc()
{
    qDebug() << "Print Screen triggered";

    if (!isGestureAllowed(AirGestureType::Fist)) {
        return;
    }

    if (m_trialActive) {
        const bool success = m_actionExecutor.executePrintScreen();
        if (success) {
            emit trialGesturePerformed(QStringLiteral("ptrsc"));
        }
        return;
    }

    m_mainWindow.hide();
    QTimer::singleShot(100, this, [this]() {
        m_actionExecutor.executePrintScreen();
    });
}

void DBusAdaptor::startTrial(const QString &gestureType)
{
    m_trialActive = true;
    m_trialGestureType = gestureTypeFromString(gestureType);
    m_mainWindow.hide();
    emit trialStateChanged();
}

void DBusAdaptor::stopTrial()
{
    if (!m_trialActive) {
        return;
    }

    m_trialActive = false;
    m_trialGestureType = AirGestureType::Unknown;
    m_mainWindow.hide();
    emit trialStateChanged();
}
