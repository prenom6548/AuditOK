/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEVFREQ_H
#define DEVFREQ_H

#include "devrpminterface.h"

#define DEV_FREQ_PATH "/sys/class/devfreq/"
#define DEV_FREQ_FILE "/governor"

class DevFreqPolicyRpm : public QObject, public DevRpmInterface
{
    Q_OBJECT
public:
    explicit DevFreqPolicyRpm(QObject *parent = nullptr);

private:
    void getDevicePowerInfo();

public:
    bool setPowerStat(int deviceIndex, const QString &stat);
    bool setAllDevicePowerStat(const QString &stat);
};

#endif // DEVFREQ_H
