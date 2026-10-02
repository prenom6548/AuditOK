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
#include "upmcpucontrol.h"
#include "../hardware-reader/hardwarereader.h"

#include <QDebug>
#include <QThread>

UpmCpuControl::UpmCpuControl(QObject *parent) : QObject(parent)
{
    m_onBattery = false; //默认电源模式
    m_cpuPolicyConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_CPUPOLICY);
    m_cpuMaxFreqConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_CPUMACFREQ);
    m_cpuMinFreqConfigMap = UpmCustomHardwareConfig::self()->getConfig(UPM_GROUP_CPUMINFREQ);

    m_cpuCoreNum = QThread::idealThreadCount();
    qDebug() << "m_cpuCoreNum :" << m_cpuCoreNum;

    m_maxCpuOnlineCore = m_cpuCoreNum - 1;
    m_cpuOnlineCore = m_maxCpuOnlineCore;

    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        m_allCpuCoreInfo.push_back(new UpmCpuCoreInfo(cpuCoreNum, this));
    }
    initCustomedModeCpuFreq();

    initSystemIdleReserveCpu0();

    initBalanceModeLimitCpuFreq();

    initAutoSwitchCpuCore();

}

void UpmCpuControl::initSystemIdleReserveCpu0()
{
    m_idleReserveCpu0[Performance] = false;
    m_idleReserveCpu0[Balance] = false;
    m_idleReserveCpu0[Powersave] = false;

    QStringList configKeys = {UPM_KEY_EXTEND_PERFORMANCE_IDEL_RESERVE_CPU0,
                              UPM_KEY_EXTEND_BALANCE_IDEL_RESERVE_CPU0,
                              UPM_KEY_EXTEND_POWERSAVE_IDEL_RESERVE_CPU0};
    QString config;
    for (int policy = Performance; policy < max_policy; ++policy) {
        if (true == UpmCustomHardwareConfig::self()->getExtendConfig(configKeys[policy], config)) {
            if (1 == config.toInt()) {
                m_idleReserveCpu0[policy] = true;
            }
        }
    }
}

void UpmCpuControl::initBalanceModeLimitCpuFreq()
{
    QString config;
    if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                UPM_KEY_EXTEND_BALANCE_MODE_REDUCE_FREQUENCY, config)) {
        if (1 == config.toInt()) {
            m_balanceModeSetMaxFreq = true;
        }
    }

    if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                UPM_KEY_EXTEND_BALANCE_MODE_MAX_FREQUENCY, config)) {
        m_balanceMaxCpuFrequency = config.toInt();
    }
}

void UpmCpuControl::initCustomedModeCpuFreq()
{
    QString config;
    bool ret = false;
    if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                UPM_KEY_EXTEND_CUSTOMED_CPU_FREQ, config)) {
        m_customedFreqMode = config.toInt();
        if (m_customedFreqMode < 0 || m_customedFreqMode > 2) {
            qDebug()<<"read error config  of custmoed...";
            m_customedFreqMode = 0;
            return;
        }
    } else {
        qDebug()<<"need't config custmoed...";
        return;
    }
    qDebug()<<"m_customedFreqMode"<<m_customedFreqMode;
    if (m_customedFreqMode == 1) {
        ret = initCustomedMaxMinModeCpuFreq();
    } else {
        ret = initCustomedSetSpeedModeCpuFreq();
    }

    if (false == ret) {
        m_customedFreqMode = 0;
    }

    initAcWatcher();
}

bool UpmCpuControl::initCustomedMaxMinModeCpuFreq()
{
    QString config;

    for (int i = 0; i < m_customedCpuFreqList.size(); ++i) {
        for (int j = 0; j < m_customedCpuFreqList[i].size(); ++j) {
            for (const QString &str : m_customedCpuFreqList[i][j]) {
                if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                            str, config)) {
                       if (str.contains("freq",Qt::CaseInsensitive)) {
                            if(!m_allCpuCoreInfo[0]->getAllAvailableFreq().contains(config)) {
                                qDebug()<<"can;t find" << config << "in" << m_allCpuCoreInfo[0]->getAllAvailableFreq();
                            }
                       } else {
                           if(!m_allCpuCoreInfo[0]->getAllAvailableGovernors().contains(config)) {
                               qDebug()<<"can't find" << config << "in" << m_allCpuCoreInfo[0]->getAllAvailableGovernors();
                               return false;
                           }
                       }
                       this->setProperty(str.toLatin1().data(), config);
                } else {
                    qDebug()<<i<<j<<"--read error--"<<str<<config;
                }
            }
        }
    }
    return true;
}

bool UpmCpuControl::initCustomedSetSpeedModeCpuFreq()
{
    QString config;
    qDebug() << __FUNCTION__;
    for (int i = 0; i < m_customedSetSpeedCpuFreqList.size(); ++i) {
        for (int j = 0; j < m_customedSetSpeedCpuFreqList[i].size(); ++j) {
            for (const QString &str : m_customedSetSpeedCpuFreqList[i][j]) {
                if (true == UpmCustomHardwareConfig::self()->getExtendConfig(
                            str, config)) {
                         qDebug() << "get" << str << ":" << config;
                       if (str.contains("freq",Qt::CaseInsensitive)) {
                            if(!m_allCpuCoreInfo[0]->getAllAvailableFreq().contains(config)) {
                                qDebug()<<"can;t find" << config << "in" << m_allCpuCoreInfo[0]->getAllAvailableFreq();
                            }
                       } else {
                           if(!m_allCpuCoreInfo[0]->getAllAvailableGovernors().contains(config)) {
                               this->setProperty(QString(UPM_KEY_EXTEND_CUSTOMED_CPU_FREQ).toLatin1().data(),false);
                               return false;
                           }
                       }
                       this->setProperty(str.toLatin1().data(), config);
                } else {
                    qDebug()<<i<<j<<"--read error--"<<str<<config;
                }
            }
        }
    }
    return true;
}

void UpmCpuControl::initAutoSwitchCpuCore()
{
    m_cpuOccupy = new UpmCpuOccupy(this);
    connect(m_cpuOccupy, &UpmCpuOccupy::cpuCoreSwitchSignal,
            this, &UpmCpuControl::dealCpuSwitchSignal);

    for (int policy = Performance; policy < max_policy; ++policy) {
        initAutoSwitchCpuCoreConfig(policy);
    }
}

void UpmCpuControl::initAutoSwitchCpuCoreConfig(int cpuPolicy)
{
    if (cpuPolicy < Performance || cpuPolicy > Powersave) {
        return ;
    }

    QStringList configKeys = {UPM_KEY_EXTEND_PERFORMANCE_AUTO_SWITCH_CPU_CORE,
                              UPM_KEY_EXTEND_BALANCE_AUTO_SWITCH_CPU_CORE,
                              UPM_KEY_EXTEND_POWERSAVE_AUTO_SWITCH_CPU_CORE};

    m_configs[cpuPolicy].configActive = false;
    m_configs[cpuPolicy].cpuSamplingInterval = 30000;
    m_configs[cpuPolicy].offlineCpuOccupy = 50;
    m_configs[cpuPolicy].onlineCpuOccupy = 70;
    m_configs[cpuPolicy].reserveCoreNum = m_cpuCoreNum / 2;

    QString config;
    m_configs[cpuPolicy].configActive =
            UpmCustomHardwareConfig::self()->getExtendConfig(configKeys[cpuPolicy], config);
    if (true == m_configs[cpuPolicy].configActive) {
        QStringList configs = config.split("-");
        if (max_auto_switch_cpu_core_config == configs.size()) {
            m_configs[cpuPolicy].cpuSamplingInterval = configs[cpu_sampling_interval].toInt();
            m_configs[cpuPolicy].offlineCpuOccupy = configs[offline_cpu_occupy].toInt();
            m_configs[cpuPolicy].onlineCpuOccupy = configs[online_cpu_occupy].toInt();
            m_configs[cpuPolicy].reserveCoreNum = configs[reserve_core_num].toInt();
            if (0 == m_configs[cpuPolicy].reserveCoreNum) {
                m_configs[cpuPolicy].reserveCoreNum = 1;
            }
        } else {
            m_configs[cpuPolicy].configActive = false;
        }
    }
}

void UpmCpuControl::initAcWatcher()
{
    QDBusConnection::systemBus().connect(
        UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, "PropertiesChanged", this, SLOT(dealAcWatcherMssage(void)));

    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        m_onBattery = reply.value().toBool();
    }
}

void UpmCpuControl::setCustomedMode(int cpuPolicy)
{
    static int lastacState = -1;
    static int lastPolicy = -1;


    if (lastacState == m_onBattery && lastPolicy == cpuPolicy) {
        return;
    }

    lastacState = m_onBattery;
    lastPolicy = cpuPolicy;

    if (m_customedFreqMode == 1) {
        setCustomedModeMaxMin(cpuPolicy);
    } else {
        setCustomedModeSetSpeed(cpuPolicy);
    }

}

void UpmCpuControl::setCustomedModeMaxMin(int cpuPolicy)
{
    QString minFreq;
    QString mode;
    QString maxFreq;

    //取出string
    maxFreq = m_customedCpuFreqList[m_onBattery][cpuPolicy][0];
    minFreq = m_customedCpuFreqList[m_onBattery][cpuPolicy][1];
    mode = m_customedCpuFreqList[m_onBattery][cpuPolicy][2];

    //通过strings取出对应的值，值已经写入到property中
    maxFreq = this->property(maxFreq.toLatin1().data()).toString();
    minFreq = this->property(minFreq.toLatin1().data()).toString();
    mode = this->property(mode.toLatin1().data()).toString();

    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        setCpuCoreOnlineState(cpuCoreNum, cpu_core_online);
        m_allCpuCoreInfo[cpuCoreNum]->setCorePolicy(mode);//先设置模式然后再设置频率，否则频率会被模式修改。
        setCoreMaxFreq(cpuCoreNum,maxFreq.toInt());
        setCoreMinFreq(cpuCoreNum,minFreq.toInt());
    }
}

void UpmCpuControl::setCustomedModeSetSpeed(int cpuPolicy)
{
    QString setFreq;
    QString mode;

    //取出string
    setFreq = m_customedSetSpeedCpuFreqList[m_onBattery][cpuPolicy][0];
    mode = m_customedSetSpeedCpuFreqList[m_onBattery][cpuPolicy][1];

    //通过strings取出对应的值，值已经写入到property中
    setFreq = this->property(setFreq.toLatin1().data()).toString();
    mode = this->property(mode.toLatin1().data()).toString();

    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        setCpuCoreOnlineState(cpuCoreNum, cpu_core_online);
        m_allCpuCoreInfo[cpuCoreNum]->setCorePolicy(mode);//先设置模式然后再设置频率，否则频率会被模式修改。
        setUserSetSpeed(cpuCoreNum, setFreq.toInt());
    }
}

int UpmCpuControl::getCpuCoreNum()
{
    return m_cpuCoreNum;
}

void UpmCpuControl::setCorePolicy(int cpuCoreId, int cpuPolicy)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    m_allCpuCoreInfo[cpuCoreId]->setCorePolicy(cpuPolicy);
}

QString UpmCpuControl::getCpuCorePolicy(int cpuCoreId)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return "cpu core id failed";
    }
    return m_allCpuCoreInfo[cpuCoreId]->getCpuCorePolicy();
}

void UpmCpuControl::setCoreMaxFreq(int cpuCoreId, int cpuFrequency)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    m_allCpuCoreInfo[cpuCoreId]->setCoreMaxFreq(cpuFrequency);
}

void UpmCpuControl::setCoreMinFreq(int cpuCoreId, int cpuFrequency)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    m_allCpuCoreInfo[cpuCoreId]->setCoreMinFreq(cpuFrequency);
}

void UpmCpuControl::setUserSetSpeed(int cpuCoreId, int cpuFrequency)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    m_allCpuCoreInfo[cpuCoreId]->setUserSetSpeed(cpuFrequency);
}

int UpmCpuControl::getDefaultCoreMaxFreq(int cpuCoreId)
{
    return m_allCpuCoreInfo[cpuCoreId]->getDefaultCoreMaxFreq();
}

void UpmCpuControl::setCoreDefaultMaxFreq(int cpuCoreId)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    m_allCpuCoreInfo[cpuCoreId]->setDefaultCoreMaxFreq();
}

void UpmCpuControl::setAllCorePolicy(int cpuPolicy)
{
    if (cpuPolicy < Performance || cpuPolicy > Powersave) {
        return ;
    }

    m_cpuPolicy = cpuPolicy;
    m_cpuOccupy->stopCalcCpuOccupy();

    if (m_customedFreqMode > 0) {
        setCustomedMode(cpuPolicy);
        qDebug() << "use customed config.";
        return;
    }


    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        setCpuCoreOnlineState(cpuCoreNum, cpu_core_online);
        if (true == m_balanceModeSetMaxFreq) {
            if (Balance == m_cpuPolicy) {
                if (0 == m_balanceMaxCpuFrequency) {
                    setCoreMaxFreq(cpuCoreNum, getDefaultCoreMaxFreq(cpuCoreNum) * 0.8);
                } else {
                    setCoreMaxFreq(cpuCoreNum, m_balanceMaxCpuFrequency);
                }
            } else {
                setCoreDefaultMaxFreq(cpuCoreNum);
            }
        }
        m_allCpuCoreInfo[cpuCoreNum]->setCorePolicy(m_cpuPolicy);
    }
    m_cpuOnlineCore = m_maxCpuOnlineCore;

    if (true == m_configs[m_cpuPolicy].configActive) {
        m_cpuOccupy->startCalcCpuOccupy(m_configs[m_cpuPolicy].cpuSamplingInterval,
                                        m_configs[m_cpuPolicy].offlineCpuOccupy,
                                        m_configs[m_cpuPolicy].onlineCpuOccupy);
    }
}

void UpmCpuControl::setAllCoreMaxFreq(int cpuFrequency)
{
    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        m_allCpuCoreInfo[cpuCoreNum]->setCoreMaxFreq(cpuFrequency);
    }
}

void UpmCpuControl::setAllCoreDefaultMaxFreq()
{
    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        m_allCpuCoreInfo[cpuCoreNum]->setDefaultCoreMaxFreq();
    }
}

bool UpmCpuControl::setAllCoreMaxFreqLevel(const QString &level)
{
    qDebug() << "UpmCpuControl::setAllCoreMaxFreqLevel: " << level;
    if (level == "max" || level == "medium" || level == "min") {
        m_cpuMaxFreqLevel = level;
        for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
            m_allCpuCoreInfo[cpuCoreNum]->setCoreMaxFreqLevel(level);
        }
        return true;
    }
    return false;
}

bool UpmCpuControl::setAllCoreMinFreqPercentage(const QString &percentage)
{
    qInfo() << "UpmCpuControl::setAllCoreMinFreqPercentage: " << percentage;
    m_cpuMinFreqPercentage = percentage;
    for (int cpuCoreNum = 0; cpuCoreNum < m_cpuCoreNum; ++cpuCoreNum) {
        m_allCpuCoreInfo[cpuCoreNum]->setCoreMinFreqPercentage(percentage);
    }
    return true;
}

int UpmCpuControl::getCpuCoreOnlineState(int cpuCoreId)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return cpu_core_error;
    }
    return m_allCpuCoreInfo[cpuCoreId]->getCpuCoreOnlineState();
}

void UpmCpuControl::setCpuCoreOnlineState(int cpuCoreId, int online)
{
    if (cpuCoreId >= m_cpuCoreNum) {
        return ;
    }
    if (0 == cpuCoreId) {
        return ;
    }

    m_allCpuCoreInfo[cpuCoreId]->setCpuCoreOnlineState(online);
}

void UpmCpuControl::systemIdleReserveCpu0(bool systmIdle)
{
    qDebug() << "systemIdleReserveCpu0 :" << systmIdle;
    if (true == systmIdle) {
        //系统进入空闲，不在通过计算cpu占用率控制开关核心，停止用于计算cpu占用率的定时器
        m_cpuOccupy->stopCalcCpuOccupy();
        qDebug() << "m_cpuPolicy :" << m_cpuPolicy << "idleReserveCpu0 :" << m_idleReserveCpu0[m_cpuPolicy];
        if (true == m_idleReserveCpu0[m_cpuPolicy]) {
            qDebug() << "1 m_cpuOnlineCore :" << m_cpuOnlineCore;
            //空闲时，仅保留一个cpu核心的功能enable，关闭其他核心
            while (m_cpuOnlineCore > 0) {
                qDebug() << "cpu core offline :" << m_cpuOnlineCore;
                setCpuCoreOnlineState(m_cpuOnlineCore, cpu_core_offline);
                m_cpuOnlineCore--;
            }
        } else {
            qDebug() << "2 m_cpuOnlineCore :" << m_cpuOnlineCore;
            //空闲时，仅保留一个cpu核心的功能disable，开启以关闭的核心
            while (m_cpuOnlineCore < m_maxCpuOnlineCore) {
                m_cpuOnlineCore++;
                qDebug() << "cpu core online :" << m_cpuOnlineCore;
                setCpuCoreOnlineState(m_cpuOnlineCore, cpu_core_online);
            }
        }
    } else {
        //非空闲状态，开启所有的cpu核心
        qDebug() << "3 m_cpuOnlineCore :" << m_cpuOnlineCore;
        while (m_cpuOnlineCore < m_maxCpuOnlineCore) {
            m_cpuOnlineCore++;
            qDebug() << "cpu core online :" << m_cpuOnlineCore;
            setCpuCoreOnlineState(m_cpuOnlineCore, cpu_core_online);
        }

        if (true == m_configs[m_cpuPolicy].configActive) {
            qDebug() << "cpu policy :" << m_cpuPolicy << "start calc cpu occupy";
            //非空闲状态，动态开关核心功能enable，重新打开计算cpu占用率的定时器
            m_cpuOccupy->startCalcCpuOccupy(m_configs[m_cpuPolicy].cpuSamplingInterval,
                                            m_configs[m_cpuPolicy].offlineCpuOccupy,
                                            m_configs[m_cpuPolicy].onlineCpuOccupy);
        }
    }
}

void UpmCpuControl::enableBalanceModeFrequency(bool enable)
{
    m_balanceModeSetMaxFreq = enable;
}

void UpmCpuControl::enableSystemIdleReserveCpu0(bool enable, int cpuPolicy)
{
    m_idleReserveCpu0[cpuPolicy] = enable;
}

void UpmCpuControl::enableAutoSwitchCpuCore(bool enable, int cpuPolicy)
{
    if (cpuPolicy < Performance || cpuPolicy > Powersave) {
            return ;
        }
    m_configs[cpuPolicy].configActive = enable;
}

void UpmCpuControl::dealCpuSwitchSignal(bool coreOnline)
{
    if (true == coreOnline) {
        if (m_cpuOnlineCore < m_maxCpuOnlineCore) {
            m_cpuOnlineCore++;
            setCpuCoreOnlineState(m_cpuOnlineCore, cpu_core_online);
        }
    } else {
        if (m_cpuOnlineCore >= m_configs[m_cpuPolicy].reserveCoreNum) {
            setCpuCoreOnlineState(m_cpuOnlineCore, cpu_core_offline);
            m_cpuOnlineCore--;
        }
    }
}

void UpmCpuControl::dealAcWatcherMssage()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        m_onBattery = reply.value().toBool();
    }

    if (m_cpuPolicy >= 0) {
        setAllCorePolicy(m_cpuPolicy);
    }
}

void UpmCpuControl::setAllCorePolicyCtl(int mode, QString scene)
{
    QString policy = nullptr;

    policy = UpmCustomHardwareConfig::self()->getScenekeyValue(mode, scene, UPM_GROUP_CPUPOLICY, getCpuPolicyConfigMap());
    qDebug() << "get policy:" << policy;
    if (policy == "0" || policy == "1" || policy == "2") {
        // 条件解释:
        // 1. 如果不是M900设备(false == isPanguM900()) -> 应用CPU控制
        // 2. 如果是M900但也是PWC30设备(true == isPgwDevice()) -> 应用CPU控制
        // 3. 只有在是M900且不是PWC30时 -> 跳过CPU控制
        // 使用HardwareReader
        HardwareReader *reader = HardwareReader::instance();
        if (false == reader->isPanguM900() || true == reader->isPgwDevice()) {
            if (policy.toInt() != m_cpuPolicy)
                setAllCorePolicy(policy.toInt());
            else 
                qInfo() << "already set policy:" << m_cpuPolicy;
        } else {
            qInfo() << "Skipping CPU control: device is M900 but not PWC30";
        }
    }
}

void UpmCpuControl::setAllCoreMaxFreqLevelCtl(int mode, QString scene)
{
    QString freqLevel = nullptr;
    freqLevel = UpmCustomHardwareConfig::self()->getScenekeyValue(mode, scene, UPM_GROUP_CPUMACFREQ, getCpuMaxFreqConfigMap());
    qDebug() << "get freqLevel:" << mode << scene << freqLevel;
    if (freqLevel == "max" || freqLevel == "medium"  || freqLevel == "min") {
        if (m_cpuMaxFreqLevel != freqLevel)
            setAllCoreMaxFreqLevel(freqLevel);
        else
            qInfo() << "already set freqLevel:" << freqLevel;
    } else {
        qWarning() << "prase config freqLevel fail:" <<  freqLevel;
    }
}

void UpmCpuControl::setAllCoreMinFreqPercentageCtl(int mode, QString scene)
{
    QString freqPercentage = nullptr;
    freqPercentage = UpmCustomHardwareConfig::self()->getScenekeyValue(mode, scene, UPM_GROUP_CPUMINFREQ, getCpuMinFreqConfigMap());
    qInfo() << "get freqPercentage:" << mode << scene << freqPercentage;
    if (m_cpuMinFreqPercentage != freqPercentage)
        setAllCoreMinFreqPercentage(freqPercentage);
    else
            qInfo() << "already set freqPercentage:" << freqPercentage;
}

QMap<QString, QString> UpmCpuControl::getCpuPolicyConfigMap()
{
    return m_cpuPolicyConfigMap;
}

QMap<QString, QString> UpmCpuControl::getCpuMaxFreqConfigMap()
{
    return m_cpuMaxFreqConfigMap;
}

QMap<QString, QString> UpmCpuControl::getCpuMinFreqConfigMap()
{
    return m_cpuMinFreqConfigMap;
}

void UpmCpuControl::printConfigMap()
{
    QMap<QString, QString>::iterator it;

    for (it = m_cpuPolicyConfigMap.begin(); it != m_cpuPolicyConfigMap.end(); ++it) {
        qDebug() << it.key() << ":" << it.value();
    }

    for (it = m_cpuMaxFreqConfigMap.begin(); it != m_cpuMaxFreqConfigMap.end(); ++it) {
        qDebug() << it.key() << ":" << it.value();
    }

    for (it = m_cpuMinFreqConfigMap.begin(); it != m_cpuMinFreqConfigMap.end(); ++it) {
        qDebug() << it.key() << ":" << it.value();
    }
}
