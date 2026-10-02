/*
 * Copyright 2021 KylinSoft Co., Ltd.
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
#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QCoreApplication>
#include <QDBusReply>
#include <QDBusMessage>
#include <QTimer>


#include "dev-power-control/devpowercontrol.h"
#include "brightness-control/brightnessnode.h"
#include "cpu-control/upmcpucontrol.h"
#include "gpu-power-control/gpupowercontrol.h"
#include "upmcustomhardwareconfig.h"
#include "hardware-reader/hardwarereader.h"


#include "powerconfig.h"
#include "upmcustomhardwareconfig.h"

#include <stdio.h>

class SysdbusRegister : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "org.ukui.powermanagement.interface")

    Q_PROPERTY(int CpuPolicy READ GetCpuPolicy NOTIFY cpuPolicyChanged)

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

private:
    /**
     * @brief controlLogin1Connection
     * 创建与login1的dbus连接
     */
    void controlLogin1Connection(QString );

    QString canControl(const QString );

    QTimer *startMoniTimer;
    void SignalMonitor();

    PowerConfig *m_powerConfig;

    BrightnessNode *m_brightnessNode;
    qint64 m_currentBrightness = -1;

    UpmCpuControl *m_upmCpuControl;
    bool m_isPanguM900 = false;
    int m_cpuPolicy;

    DevPowerControl *m_devPowerControl;

    GpuPowerControl *m_gpuPowerControl;


    QString m_currentScene;

    void handleScene(const QString &scene);

    QString getCurrentScene();

    enum Scene
    {
        DEFAULT = 0,
        IDLE,
        LONGIDLE,
        MEETING,
        OFFICEPPT,
        OFFICEWORD,
        OFFICEEXCEL,
        VIDEO,
        NO_REMOTE_SESSION,
        IDLE_LOW_LOAD,
        PPT_BUSY,
        PPT_IDLE,
        WORD_BUSY,
        WORD_IDLE,
        EXCEL_BUSY,
        EXCEL_IDLE,
        VIDEO_BUSY,
        VIDEO_IDLE,
        IDLE_LOW_POWER,
    };
    QStringList m_Scene = {"default", "idle", "longidle", "meeting", "officePpt", "officeWord", "officeExcel", "video","noremotesession","idlelowload","pptBusy","pptIdle","wordBusy","wordIdle","excelBusy","excelIdle","videoBusy","videoIdle","idlelowpower"};

signals:
    void cpuPolicyChanged(int);

private slots:
    void onSceneChanged(const QStringList &scenes);

    bool checkEnviron(int pid);

public slots:
    Q_SCRIPTABLE bool CanSetBrightness();

    Q_SCRIPTABLE QString RegulateBrightness(qulonglong);

    Q_SCRIPTABLE QString resetBrightness();

    Q_SCRIPTABLE void CpuFrequencyModulation(int);

    Q_SCRIPTABLE void GpuFrequencyModulation(const int);

    Q_SCRIPTABLE qulonglong GetMaxBrightness();

    Q_SCRIPTABLE qulonglong GetBrightness();

    /**
     * @brief IsBrightnessEnhancement‌
     * 是否支持亮度增强
     * false 不支持（默认）；true 支持
     * @return
     */
    Q_SCRIPTABLE bool IsBrightnessEnhancementSupported();

    Q_SCRIPTABLE void SetDevicePowerCtl(int);

    Q_SCRIPTABLE int GetCpuPolicy();

    Q_SCRIPTABLE void SystemIdleReserveCpu0(bool);

    Q_SCRIPTABLE void EnableReduceCpuFrequency(bool);

    Q_SCRIPTABLE void EnableSystemIdleReserveCpu0(bool, int);

    Q_SCRIPTABLE void EnableAutoSwitchCpuCore(bool, int);

    Q_SCRIPTABLE int GetCpuCoreNum();

    Q_SCRIPTABLE QString GetCpuCorePolicy(int);

    Q_SCRIPTABLE bool FirstRunDonotSuspend();

    Q_SCRIPTABLE bool isFlemingXModel();
private:
    Q_PROPERTY(QString IsBrightnessEnhancementSupported READ IsBrightnessEnhancementSupported)
};

#endif // SYSDBUSREGISTER_H
