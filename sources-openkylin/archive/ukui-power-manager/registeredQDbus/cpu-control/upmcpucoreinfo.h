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
#ifndef UPMCPUCOREINFO_H
#define UPMCPUCOREINFO_H

#include <QObject>
#include <QDebug>

#define CPU_DEVICES_PATH                   "/sys/devices/system/cpu/cpu%1/"
#define CPU_CORE_ONLINE                    "online"
#define CPU_DEVICES_FREQ_PATH              "/sys/devices/system/cpu/cpu%1/cpufreq/"
#define CPU_SCALING_GOVERNORS              "scaling_governor"
#define CPU_SCALING_AVAILABLE_GOVERNORS    "scaling_available_governors"
#define CPU_SCALING_MAX_FREQ               "scaling_max_freq"
#define CPU_SCALING_MIN_FREQ               "scaling_min_freq"
#define CPU_SCALING_AVAILABLE_FREQ         "scaling_available_frequencies"

#define CPU_SCALING_SETSPEED               "scaling_setspeed"

#define CPU_AVAILABLE_FREQ                  "scaling_available_frequencies"
#define CPU_AVAILABLE_GOVERNORS             "scaling_available_governors"
enum cpu_core_online_state
{
    cpu_core_offline = 0,
    cpu_core_online,
    cpu_core_error,
};

enum policy
{
    Performance = 0,
    Balance,
    Powersave,
    max_policy
};

class UpmCpuCoreInfo : public QObject
{
    Q_OBJECT
public:
    explicit UpmCpuCoreInfo(int cpuCoreId, QObject *parent = nullptr);

    int getCpuCoreOnlineState(void);
    void setCpuCoreOnlineState(int online);

    void setCorePolicy(int cpuPolicy);
    void setCorePolicy(QString cpuPolicy);
    void setCoreMaxFreq(int cpuFrequency);
    void setCoreMinFreq(int cpuFrequency);
    int getDefaultCoreMaxFreq();
    void setDefaultCoreMaxFreq();
    void setCoreMaxFreqLevel(const QString &level);
    void setCoreMinFreqPercentage(const QString &percentage);

    void setUserSetSpeed(int cpuFrequency);
    QString getCpuCorePolicy();

    QStringList getAllAvailableFreq();
    QStringList getAllAvailableGovernors();

private:
    int m_cpuCoreId = -1;
    int m_defaultMaxFreq = 0;
    int m_defaultMinFreq = 0;
    QStringList m_avaiableMode;
    QStringList m_avaiableFreq;
    QStringList m_coreFreqMode = {"performance", "powersave", "powersave"};
    //QMap<QString, QString> m_cpuPolicyMap;
signals:

};

#endif // UPMCPUINFO_H
