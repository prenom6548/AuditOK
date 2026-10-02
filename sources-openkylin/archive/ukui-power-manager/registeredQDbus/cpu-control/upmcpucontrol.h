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
#ifndef UPMCPUCONTROL_H
#define UPMCPUCONTROL_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include "upmcpucoreinfo.h"
#include "upmcpuoccupy.h"
#include "../upmcustomhardwareconfig.h"


#define UPOWER_PATH                         "/org/freedesktop/UPower"
#define UPOWER_SERVICE                      "org.freedesktop.UPower"
#define UPOWER_DISPLAY_PATH                 "/org/freedesktop/UPower/devices/DisplayDevice"
#define FREEDESKTOP_UPOWER                  "org.freedesktop.DBus.Properties"

enum em_auto_switch_cpu_core_config
{
    cpu_sampling_interval = 0,
    offline_cpu_occupy,
    online_cpu_occupy,
    reserve_core_num,
    max_auto_switch_cpu_core_config
};

typedef struct stu_auto_switch_cpu_core_config
{
    int cpuSamplingInterval;
    int offlineCpuOccupy;
    int onlineCpuOccupy;
    int reserveCoreNum;
    bool configActive;
}AutoSwitchCpuCoreConfig;

class UpmCpuControl : public QObject
{
    Q_OBJECT
public:
    explicit UpmCpuControl(QObject *parent = nullptr);

    int getCpuCoreNum();
    void setCorePolicy(int cpuCoreId, int cpuPolicy);
    QString getCpuCorePolicy(int cpuCoreId);
    void setCoreMaxFreq(int cpuCoreId, int cpuFrequency);
    void setCoreMinFreq(int cpuCoreId, int cpuFrequency);
    void setUserSetSpeed(int cpuCoreId, int cpuFrequency);

    int getDefaultCoreMaxFreq(int cpuCoreId);
    void setCoreDefaultMaxFreq(int cpuCoreId);

    void setAllCorePolicy(int cpuPolicy);
    void setAllCoreMaxFreq(int cpuFrequency);
    void setAllCoreDefaultMaxFreq();
    bool setAllCoreMaxFreqLevel(const QString &level); //3个档： max medium min
    bool setAllCoreMinFreqPercentage(const QString &percentage);
    bool setCoreMaxFreqLevel(int cpuCoreId, const QString &level);

    int getCpuCoreOnlineState(int cpuCoreId);
    void setCpuCoreOnlineState(int cpuCoreId, int online);

    void systemIdleReserveCpu0(bool systmIdle);
    void enableBalanceModeFrequency(bool enable);
    void enableSystemIdleReserveCpu0(bool enable, int cpuPolicy);
    void enableAutoSwitchCpuCore(bool enable, int cpuPolicy);

     //设置场景下的值
    void setAllCorePolicyCtl(int mode, QString scene);
    void setAllCoreMaxFreqLevelCtl(int mode, QString scene);
    void setAllCoreMinFreqPercentageCtl(int mode, QString scene);
    void printConfigMap();
    QMap<QString, QString> getCpuPolicyConfigMap();
    QMap<QString, QString> getCpuMaxFreqConfigMap();
    QMap<QString, QString> getCpuMinFreqConfigMap();

private:
    int m_cpuCoreNum;
    QString m_cpuMaxFreqLevel;
    QString m_cpuMinFreqPercentage;
    int m_cpuPolicy = -1;
    QVector<UpmCpuCoreInfo *> m_allCpuCoreInfo;
    int m_balanceMaxCpuFrequency = 0;
    bool m_balanceModeSetMaxFreq = false;

    bool m_idleReserveCpu0[max_policy];

    UpmCpuOccupy *m_cpuOccupy;
    AutoSwitchCpuCoreConfig m_configs[max_policy];
    int m_maxCpuOnlineCore;
    int m_cpuOnlineCore;
    int m_onBattery;//0 AC,1 DC
    int m_customedFreqMode = 0; //1 写入 max min节点，2写入 setspeed节点
    QList<QStringList> m_customedConfigList;
    const QList<QList<QStringList>> m_customedCpuFreqList ={{//AC max.min.mode位置不能变
                                                       {UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MAX_FREQ_AC,
                                                        UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MIN_FREQUENCY_AC,
                                                        UPM_KEY_EXTEND_PERFORMANCE_MODE_AC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MAX_FREQ_AC,
                                                        UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MIN_FREQUENCY_AC,
                                                        UPM_KEY_EXTEND_BALANCE_MODE_AC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MAX_FREQ_AC,
                                                        UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MIN_FREQUENCY_AC,
                                                        UPM_KEY_EXTEND_SAVE_MODE_AC}
                                                      },
                                                      {//DC
                                                       {UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MAX_FREQ_DC,
                                                        UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MIN_FREQUENCY_DC,
                                                        UPM_KEY_EXTEND_PERFORMANCE_MODE_DC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MAX_FREQ_DC,
                                                        UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MIN_FREQUENCY_DC,
                                                        UPM_KEY_EXTEND_BALANCE_MODE_DC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MAX_FREQ_DC,
                                                        UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MIN_FREQUENCY_DC,
                                                        UPM_KEY_EXTEND_SAVE_MODE_DC}
                                                      }};


    QMap<QString, QString> m_cpuPolicyConfigMap;
    QMap<QString, QString> m_cpuMaxFreqConfigMap;
    QMap<QString, QString> m_cpuMinFreqConfigMap;

    const QList<QList<QStringList>> m_customedSetSpeedCpuFreqList ={{//AC max.min.mode位置不能变
                                                       {UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_FREQ_AC,
                                                        UPM_KEY_EXTEND_PERFORMANCE_MODE_AC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_FREQ_AC,
                                                        UPM_KEY_EXTEND_BALANCE_MODE_AC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_FREQ_AC,
                                                        UPM_KEY_EXTEND_SAVE_MODE_AC}
                                                      },
                                                      {//DC
                                                       {UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_FREQ_DC,
                                                        UPM_KEY_EXTEND_PERFORMANCE_MODE_DC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_FREQ_DC,
                                                        UPM_KEY_EXTEND_BALANCE_MODE_DC},
                                                       {UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_FREQ_DC,
                                                        UPM_KEY_EXTEND_SAVE_MODE_DC}
                                                      }};

    void initSystemIdleReserveCpu0();

private:
    void initBalanceModeLimitCpuFreq();
    /** 初始化定制cpu管控的功能。
     * @brief initCustomedModeLimitCpuFreq
     */
    void initCustomedModeCpuFreq();
    bool initCustomedMaxMinModeCpuFreq();
    bool initCustomedSetSpeedModeCpuFreq();

    void initAutoSwitchCpuCore();
    void initAutoSwitchCpuCoreConfig(int cpuPolicy);
    void initAcWatcher();
    void setCustomedMode(int cpuPolicy);
    void setCustomedModeMaxMin(int cpuPolicy);
    void setCustomedModeSetSpeed(int cpuPolicy);
private slots:
    void dealCpuSwitchSignal(bool);
    void dealAcWatcherMssage();

signals:

};

#endif // UPMCPUCONTROL_H
