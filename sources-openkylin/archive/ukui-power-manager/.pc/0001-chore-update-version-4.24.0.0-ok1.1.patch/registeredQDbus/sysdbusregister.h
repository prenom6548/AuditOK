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

#define TLP_ORIGIN_CONF             "/usr/share/ukui-power-manager/tlp/origin.conf"
#define TLP_ORIGIN_DEFAULT_CONFIG   "/usr/share/ukui-power-manager/tlp/upm_origin.conf"
#define TLP_ORIGIN_GET_PATH_SH      "/usr/share/ukui-power-manager/tlp/get_tlp_bat_paths.sh"

#define TLP_SET_WHITE_LIST            "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/tlp"
#define TLP_SET_WHITE_LIST1           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/05-tlp-func-pm"
#define TLP_SET_WHITE_LIST2           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/10-tlp-func-cpu"
#define TLP_SET_WHITE_LIST3           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/15-tlp-func-disk"
#define TLP_SET_WHITE_LIST4           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/20-tlp-func-usb"
#define TLP_SET_WHITE_LIST5           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/25-tlp-func-rf"
#define TLP_SET_WHITE_LIST6           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/30-tlp-func-rf-sw"
#define TLP_SET_WHITE_LIST7           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/35-tlp-func-batt"
#define TLP_SET_WHITE_LIST8           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/40-tlp-func-bay"
#define TLP_SET_WHITE_LIST9           "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/45-tlp-func-gpu"
#define TLP_SET_WHITE_LIST10          "kysec_set -n exectl -v verified /usr/share/ukui-power-manager/tlp/func.d/tlp-func-stat"


#include "dev-power-control/devpowercontrol.h"
#include "brightness-control/brightnessnode.h"
#include "cpu-control/upmcpucontrol.h"
#include "gpu-power-control/gpupowercontrol.h"
#include "upmcustomhardwareconfig.h"
#include "hardware-reader/hardwarereader.h"


#define TLP_BAT_CMD                  "/usr/share/ukui-power-manager/tlp/tlp bat"
#include "powerconfig.h"
#include "upmcustomhardwareconfig.h"

#include <stdio.h>

class SysdbusRegister : public QObject

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

    QProcess *m_progress;
    QMap<QString, QString> m_tlpMap;

    bool checkEnviron(int pid);

    /** 检查tlp默认配置是否存在
     * @brief checkTlpOrigin
     */
    void checkTlpOrigin();

    /** 读取所有的默认节点写入到配置文件内。
     * @brief genDefaultConfig
     */
    bool genDefaultConfig();

    /** 读取缺省值到map内。
     * @brief readAllNodeToMap
     */
    bool readAllNodeToMap();

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

    Q_SCRIPTABLE void enterTlp();

    Q_SCRIPTABLE void exitTlp();

    Q_SCRIPTABLE QString printfTlpNodeValue();

    Q_SCRIPTABLE int GetCpuCoreNum();

    Q_SCRIPTABLE QString GetCpuCorePolicy(int);

    Q_SCRIPTABLE bool FirstRunDonotSuspend();

    Q_SCRIPTABLE bool isFlemingXModel();
private:
    Q_PROPERTY(QString IsBrightnessEnhancementSupported READ IsBrightnessEnhancementSupported)
};

#endif // SYSDBUSREGISTER_H
