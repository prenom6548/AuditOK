/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef UPMCUSTOMHARDWARECONFIG_H
#define UPMCUSTOMHARDWARECONFIG_H

#include <QObject>
#include <QSettings>
#include <QMap>

#define UPM_NO_THIS_KEY    QStringLiteral("no-this-key")

#define UPM_HARDWARE_INFO_FILE                QStringLiteral("/sys/class/dmi/id/modalias")
#define UPM_CPU_INFO_FILE                     QStringLiteral("/proc/cpuinfo")
#define UPM_GLOBAL_HARDWARE_CONFIG_FILE       QStringLiteral("/opt/system/resource/ukui/ukui-power-manager/upm-hardware-global.conf")
#define UPM_EXTEND_HARDWARE_CONFIG_PATH       QStringLiteral("/opt/system/resource/ukui/ukui-power-manager/upm-hardware-extend.d/")
#define UPM_EXTEND_HARDWARE_ARCH_CONFIG_PATH       QStringLiteral("/opt/system/resource/ukui-power-manager/upm-hardware-extend.d/Arch.config")

#define UPM_GROUP_EXTEND_BRAND_GENERIC_CONFIG   QStringLiteral("Brand-generic-config")
#define UPM_KEY_FIRST_RUN_DONOT_SUSPEND         QStringLiteral("firstRunDonotSuspend")

#define UPM_KEY_BOARD_NAME                    QStringLiteral("boardName")

#define UPM_GROUP_THIS_PRODUCT                QStringLiteral("this_product")
#define UPM_KEY_FIRST_RUN                     QStringLiteral("firstRun")
#define UPM_KEY_EXTEND_CONFIG_PATH            QStringLiteral("extendConfigPath")
#define UPM_KEY_DEVICE_IDENTIFICATION         QStringLiteral("deviceIdentification")

#define UPM_GROUP_IGNORE_BRIGHTNESS_NODE      QStringLiteral("ignore-brightness-node")

#define UPM_GROUP_HARDWARE_CONTROL                     QStringLiteral("hardware-control")
#define UPM_GROUP_CPUPOLICY                            QStringLiteral("cpuPolicy")
#define UPM_GROUP_CPUMACFREQ                           QStringLiteral("cpuMaxFreq")
#define UPM_GROUP_CPUMINFREQ                           QStringLiteral("cpuMinFreqPercentage")
#define UPM_GROUP_DEVFREQPOLICY                        QStringLiteral("devfreqPolicy")
#define UPM_GROUP_DEVFREQMAX                           QStringLiteral("devfreqMax")
#define UPM_GROUP_GPUPOLICY                            QStringLiteral("gpuPolicy")
#define UPM_GROUP_USBRPM                               QStringLiteral("usbRPM")
#define UPM_GROUP_PCIRPM                               QStringLiteral("pciRPM")
#define UPM_GROUP_I2CRPM                               QStringLiteral("i2cRPM")
#define UPM_GROUP_SATARPM                              QStringLiteral("sataRPM")
#define UPM_GROUP_PCIEASPM                             QStringLiteral("pcieAspm")
#define UPM_GROUP_SATALINKPOWER                        QStringLiteral("sataLinkPower")
#define UPM_GROUP_SNAHDAINTELAUDIO                     QStringLiteral("sndHdaIntel")
#define UPM_GROUP_NETWOl                               QStringLiteral("netWOl")
#define UPM_GROUP_WAKEUPSOURCE                         QStringLiteral("wakeupSource")
#define UPM_GROUP_KEYBOARDBACKLIGHT                    QStringLiteral("keyBoardBacklight")
#define UPM_GROUP_TOUCHSCREEN                          QStringLiteral("touchScreen")

#define FILTER_USB_CLASS                               QStringLiteral("usbRPMClassBasedFilter")
#define FILTER_PCI_CLASS                               QStringLiteral("pciRPMClassBasedFilter")
#define IGNORE_USB_DEV                                 QStringLiteral("usbRPMDevIgnore")

#define UPM_GROUP_KEYVALUE_NOTCONFIG                   QStringLiteral("default")

#define UPM_KEY_EXTEND_BRIGHTNESS_NODE                    QStringLiteral("brightnessNode")

#define UPM_KEY_EXTEND_PERFORMANCE_IDEL_RESERVE_CPU0      QStringLiteral("performanceIdleReserveCpu0")
#define UPM_KEY_EXTEND_BALANCE_IDEL_RESERVE_CPU0          QStringLiteral("balanceIdleReserveCpu0")
#define UPM_KEY_EXTEND_POWERSAVE_IDEL_RESERVE_CPU0        QStringLiteral("powersaveIdleReserveCpu0")


/*定制状态 AC start*/
//AC start
//是否定制频率
#define UPM_KEY_EXTEND_CUSTOMED_CPU_FREQ         QStringLiteral("CustomedCpuFreq")


//电源定制性能设置值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_FREQ_AC         QStringLiteral("customedPerformanceFreqAc")
//电源定制平衡设置值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_FREQ_AC         QStringLiteral("customedBalanceFreqAc")
//电源定制节能设置值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_FREQ_AC         QStringLiteral("customedPowerSaveFreqAc")

//电池定制性能设置值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_FREQ_DC         QStringLiteral("customedPerformanceFreqDc")
//电池定制平衡设置值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_FREQ_DC         QStringLiteral("customedBalanceFreqDc")
//电池定制节能设置值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_FREQ_DC         QStringLiteral("customedPowerSaveFreqDc")


//定制平衡最大值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MAX_FREQ_AC         QStringLiteral("customedBalanceMaxFreqAc")
//定制平衡最小值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MIN_FREQUENCY_AC         QStringLiteral("customedBlanceMinFreqAc")
//定制平衡模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_BALANCE_MODE_AC         QStringLiteral("balanceModeAc")

//定制性能最大值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MAX_FREQ_AC         QStringLiteral("customedPerformanceMaxFreqAc")
//定制性能最小值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MIN_FREQUENCY_AC         QStringLiteral("customedPerformanceMinFreqAc")
//定制性能模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_PERFORMANCE_MODE_AC         QStringLiteral("performanceModeAc")

//定制节能最大值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MAX_FREQ_AC         QStringLiteral("customedPowerSaveMaxFreqAc")
//定制节能最小值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MIN_FREQUENCY_AC         QStringLiteral("customedPowerSaveMinFreqAc")
//定制节能模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_SAVE_MODE_AC         QStringLiteral("powerSaveModeAc")

//DC start
//定制平衡最大值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MAX_FREQ_DC         QStringLiteral("customedBalanceMaxFreqDc")
//定制平衡最小值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MIN_FREQUENCY_DC         QStringLiteral("customedBlanceMinFreqDc")
//定制平衡模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_BALANCE_MODE_DC         QStringLiteral("balanceModeDc")

//定制性能最大值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MAX_FREQ_DC         QStringLiteral("customedPerformanceMaxFreqDc")
//定制性能最小值
#define UPM_KEY_EXTEND_CUSTOMED_PERFORMANCE_MODE_MIN_FREQUENCY_DC         QStringLiteral("customedPerformanceMinFreqDc")
//定制性能模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_PERFORMANCE_MODE_DC         QStringLiteral("performanceModeDc")

//定制节能最大值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MAX_FREQ_DC         QStringLiteral("customedPowerSaveMaxFreqDc")
//定制节能最小值
#define UPM_KEY_EXTEND_CUSTOMED_SAVE_MODE_MIN_FREQUENCY_DC         QStringLiteral("customedPowerSaveMinFreqDc")
//定制节能模式，模式节点写入的模式关键字
#define UPM_KEY_EXTEND_SAVE_MODE_DC         QStringLiteral("powerSaveModeDc")


//定制平衡最大值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MAX_FREQUENCY         QStringLiteral("customedBalanceMaxFreq")
//定制平衡最小值
#define UPM_KEY_EXTEND_CUSTOMED_BALANCE_MODE_MIN_FREQUENCY         QStringLiteral("customedBlanceMinFreq")

/*定制状态 end*/

/*平衡模式  定义开始*/
//平衡模式下空闲调整频率

#define UPM_KEY_EXTEND_BALANCE_MODE_REDUCE_FREQUENCY      QStringLiteral("balanceModeReduceFrequency")

//平衡模式最大频率
#define UPM_KEY_EXTEND_BALANCE_MODE_MAX_FREQUENCY         QStringLiteral("balanceModeMaxFrequency")
/*定制平衡模式  定义结束*/



#define UPM_KEY_EXTEND_PERFORMANCE_AUTO_SWITCH_CPU_CORE   QStringLiteral("performanceAutoSwitchCpuCore")
#define UPM_KEY_EXTEND_BALANCE_AUTO_SWITCH_CPU_CORE       QStringLiteral("balanceAutoSwitchCpuCore")
#define UPM_KEY_EXTEND_POWERSAVE_AUTO_SWITCH_CPU_CORE     QStringLiteral("powersaveAutoSwitchCpuCore")
#define UPM_KEY_EXTEND_BUS_DEVICE_AUTO_SUSPEND            QStringLiteral("busDeviceAutoSuspend")
#define UPM_KEY_EXTEND_GPU_FREQUENCY_STATE                QStringLiteral("gpuFrequencyState")
#define UPM_KEY_EXTEND_GPU_FREQUENCY_LEVEL                QStringLiteral("gpuFrequencyLevel")
#define UPM_KEY_EXTEND_INTEL_AUDIO_MODE                   QStringLiteral("intelAudioMode")

class UpmCustomHardwareConfig : public QObject
{
    Q_OBJECT
public:
    explicit UpmCustomHardwareConfig(QObject *parent = nullptr);
    ~UpmCustomHardwareConfig();
    static UpmCustomHardwareConfig *self();

    bool isPanguM900();
    bool isHygonCpu();
    bool isZhaoxinCpu();
    bool isD3000M();
    bool isMipsCpu();
    bool brightnessNodeEffective();
    QString getBrightnessNode();
    bool firstRunDonotSuspend();
    bool getExtendConfig(const QString &configKey, QString &config);
    //获取本机的配置，定制配置覆盖全局配置
    //参数传配置文件的Group名字
    QMap<QString, QString> getConfig(const QString &group);
    QString getScenekeyValue(int policy, QString scene, QString group, QMap<QString, QString> iniMap);

private:
    QString m_hardwareInfo = "";
    QString m_cpuType = "";
    QSettings *m_globalConfig = nullptr;
    QSettings *m_extendConfig = nullptr;
    QString m_productGroup = "";
    QString m_extendConfigPath = "";

    void getHardwareInfo();
    void getCpuType();

    bool readConfig(QSettings *settings, const QString &group,
                    const QString &key, QVariant &value);
    bool getExtendConfigFile(QString &configFilePath);
    bool readAll(const QString &filePath, QString &content);
signals:

};

#endif // UPMCUSTOMDEFAULTHARDWARECONFIG_H
