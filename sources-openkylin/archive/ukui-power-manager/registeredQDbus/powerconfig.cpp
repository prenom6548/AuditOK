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
 */
#include "powerconfig.h"

#include <QFile>

PowerConfig::PowerConfig(QObject *parent) : QObject(parent)
{
    m_qsettings = new QSettings(POWER_MANAGER_CONF_PATH, QSettings::NativeFormat);
    QVariant value;
    if (true == getPowerConfig(POWER_MANAGER_RUN_TIMES, value)) {
        m_powerManagerRunTimes = value.toInt();
    }
    m_powerManagerRunTimes++;
    updatePowerConfig(POWER_MANAGER_RUN_TIMES, QString::number(m_powerManagerRunTimes));
}

PowerConfig::~PowerConfig() {}

int PowerConfig::getPowerManagerRunTimes()
{
    return m_powerManagerRunTimes;
}

void PowerConfig::updatePowerConfig(const QString &key, const QString &value)
{
    m_qsettings->sync();
    m_qsettings->beginGroup(CONFIG_GROUP_CONFIG);
    m_qsettings->setValue(key, value);
    m_qsettings->endGroup();
    m_qsettings->sync();
}

bool PowerConfig::getPowerConfig(const QString &key, QVariant &value)
{
    m_qsettings->beginGroup(CONFIG_GROUP_CONFIG);
    value = m_qsettings->value(key, POWER_MANAGER_NO_THIS_KEY);
    m_qsettings->endGroup();

    if (POWER_MANAGER_NO_THIS_KEY == value.toString()) {
        return false;
    }
    return true;
}
