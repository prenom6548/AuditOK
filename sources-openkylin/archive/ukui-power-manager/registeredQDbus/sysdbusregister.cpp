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

#include "sysdbusregister.h"

#include <QDebug>
#include <QString>
#include <QStringList>

SysdbusRegister::SysdbusRegister()
{
    m_powerConfig = new PowerConfig(this);

    m_brightnessNode = new BrightnessNode(this);

    m_upmCpuControl = new UpmCpuControl(this);

    m_devPowerControl = new DevPowerControl(this);  //配置硬件

    m_gpuPowerControl = new GpuPowerControl(this); //配置硬件GPU 后续把名字改过来

    startMoniTimer = new QTimer(this);
    startMoniTimer->setInterval(1000*1); //防止硬件初始化未完成就接收信号
    connect(startMoniTimer, &QTimer::timeout, this, &SysdbusRegister::SignalMonitor);
    startMoniTimer->start();
}

SysdbusRegister::~SysdbusRegister() {}

void SysdbusRegister::SignalMonitor()
{
    startMoniTimer->stop();
    QDBusConnection connection = QDBusConnection::systemBus();
    bool connected = connection.connect(
        "com.kylin.ProcessManagerDaemon",
        "/com/kylin/ProcessManagerDaemon",
        "com.kylin.ProcessManagerDaemon",
        "SceneChanged",
        this,
        SLOT(onSceneChanged(QStringList))
    );
    if (!connected) {
        qWarning() << "connnet ProcessManagerDaemon dbus fail";
    } else {
        qDebug() << "connnet ProcessManagerDaemon dbus success";
    }
}

void SysdbusRegister::onSceneChanged(const QStringList &scenes)
{
        qDebug() << "receive SceneChanged single";
        
        QString currentScene = "";
        if (!scenes.isEmpty()) {
            currentScene = scenes[0];          
            m_currentScene = currentScene;
            handleScene(currentScene);
        }
}

void SysdbusRegister::handleScene(const QString &scene)
{
    qDebug() << "handle scene:" << scene;
    QString current_scene;

    if (scene == "default") {
        qInfo() << "change to default scene";
        current_scene = m_Scene[DEFAULT];
    }
    else if (scene == "idle") {
        qInfo() << "change to idle scene";
        current_scene = m_Scene[IDLE];
    }
    else if (scene.contains("idle")  && scene.contains("no_remote_session")) {
        qInfo() << "change to no_remote_session scene";
        current_scene = m_Scene[NO_REMOTE_SESSION];
    }
    else if (scene.contains("idle")  && scene.contains("idle_low_Load")) {
        qInfo() << "change to idle_low_Load scene";
        current_scene = m_Scene[IDLE_LOW_LOAD];
    }
    else if (scene.contains("idle")  && scene.contains("idle_low_power")) {
        qInfo() << "change to idle_low_power scene";
        current_scene = m_Scene[IDLE_LOW_POWER];
    }
    else if (scene == "log_idle") {
        qInfo() << "change to log_idle scene";
        current_scene = m_Scene[LONGIDLE];
    }
    else if (scene == "meeting") {
        qInfo() << "change to meeting scene";
        current_scene = m_Scene[MEETING];
    }
    else if (scene == "office_ppt") {
        qInfo() << "change to office_ppt scene";
        current_scene = m_Scene[OFFICEPPT];
    }
    else if (scene.contains("office_ppt")  && scene.contains("ppt_busy")) {
        qInfo() << "change to ppt_busy scene";
        current_scene = m_Scene[PPT_BUSY];
    }
    else if (scene.contains("office_ppt")  && scene.contains("ppt_idle")) {
        qInfo() << "change to ppt_idle scene";
        current_scene = m_Scene[PPT_IDLE];
    }
    else if (scene == "office_word") {
        qInfo() << "change to office_word scene";
        current_scene = m_Scene[OFFICEWORD];
    }
    else if (scene.contains("office_word")  && scene.contains("word_busy")) {
        qInfo() << "change to word_busy scene";
        current_scene = m_Scene[WORD_BUSY];
    }
    else if (scene.contains("office_word")  && scene.contains("word_idle")) {
        qInfo() << "change to word_idle scene";
        current_scene = m_Scene[WORD_IDLE];
    }
    else if (scene == "office_excel") {
        qInfo() << "change to office_excel scene";
        current_scene = m_Scene[OFFICEEXCEL];
    }
    else if (scene.contains("office_excel")  && scene.contains("excel_busy")) {
        qInfo() << "change to excel_busy scene";
        current_scene = m_Scene[EXCEL_BUSY];
    }
    else if (scene.contains("office_excel")  && scene.contains("excel_idle")) {
        qInfo() << "change to excel_idle scene";
        current_scene = m_Scene[EXCEL_IDLE];
    }
    else if (scene == "video") {
        qInfo() << "change to video scene";
        current_scene = m_Scene[VIDEO];
    }
    else if (scene.contains("video")  && scene.contains("video_busy")) {
        qInfo() << "change to video_busy scene";
        current_scene = m_Scene[VIDEO_BUSY];
    }
    else if (scene.contains("video")  && scene.contains("video_idle")) {
        qInfo() << "change to video_idle scene";
        current_scene = m_Scene[VIDEO_IDLE];
    }
    else {
        qWarning() << "unkown scene:" << scene;
    }
    //设置硬件
    m_devPowerControl->SetDevicePowerCtl(m_cpuPolicy, current_scene);
    m_upmCpuControl->setAllCorePolicyCtl(m_cpuPolicy, current_scene);
    m_upmCpuControl->setAllCoreMaxFreqLevelCtl(m_cpuPolicy, current_scene);
    m_upmCpuControl->setAllCoreMinFreqPercentageCtl(m_cpuPolicy, current_scene);
}

// 获取当前场景（供其他函数使用）
QString SysdbusRegister::getCurrentScene()
{
    return m_currentScene;
}

bool SysdbusRegister::CanSetBrightness()
{
    return m_brightnessNode->canSetBrightness();
}

bool SysdbusRegister::checkEnviron(int pid)
{
    QString filePath = QString("/proc/%1/environ").arg(pid);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "file.open failed" << filePath;
        return false;
    }
    QTextStream in(&file);
    QString data = in.readAll();
    file.close();
    data.replace('\0', '\n');
    QStringList environVars = data.split('\n');
    for (const QString &var : environVars) {
        if (!var.isEmpty()) {
            int pos = var.indexOf('=');
            if (pos != -1) {
                QString key = var.left(pos);
                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT") {
                    return false;
                }
            }
        }
    }
    return true;
}

qulonglong SysdbusRegister::GetMaxBrightness()
{
    if (false == m_brightnessNode->canSetBrightness()) {
        return 0;
    }

    return m_brightnessNode->getMaxBrightness();
}

qulonglong SysdbusRegister::GetBrightness()
{

    if (false == m_brightnessNode->canSetBrightness()) {
        return 0;
    }

    return m_brightnessNode->getCurrentBrightness();
}

QString SysdbusRegister::RegulateBrightness(qulonglong brightness)
{
    if (false == m_brightnessNode->canSetBrightness()) {
        return "no effective node";
    }

    m_currentBrightness = brightness;
    return m_brightnessNode->setCurrentBrightness(brightness);
}

QString SysdbusRegister::resetBrightness()
{
    if (m_currentBrightness < 0){
        return "unset";
    }
    if (false == m_brightnessNode->canSetBrightness()) {
        return "no effective node";
    }

    return m_brightnessNode->setCurrentBrightness(m_currentBrightness);
}

bool SysdbusRegister::IsBrightnessEnhancementSupported()
{
    return m_brightnessNode->isBrightnessEnhancementSupported();
}

void SysdbusRegister::CpuFrequencyModulation(int strategy)
{
    static int lastCpuPolicy = -1; // 用于记录上一次的策略

    // 如果当前策略与上一次相同，则不发射信号
    if (lastCpuPolicy == strategy) {
        qDebug() << "CpuFrequencyModulation: No change in CPU policy, skipping signal emission.";
    } else {
        emit cpuPolicyChanged(strategy);
        qDebug() << "CpuFrequencyModulation: CPU policy changed, signal emitted.";
    }

    // 更新上一次的策略
    lastCpuPolicy = strategy;

    m_cpuPolicy = strategy;

    // 将这部分功能移到setAllCorePolicyCtl() 这个接口中
    // // 条件解释:
    // // 1. 如果不是M900设备(false == isPanguM900()) -> 应用CPU控制
    // // 2. 如果是M900但也是PWC30设备(true == isPgwDevice()) -> 应用CPU控制
    // // 3. 只有在是M900且不是PWC30时 -> 跳过CPU控制
    // // 使用HardwareReader
    // HardwareReader *reader = HardwareReader::instance();
    // if (false == reader->isPanguM900() || true == reader->isPgwDevice()) {
    //     m_upmCpuControl->setAllCorePolicy(strategy);
    // } else {
    //     qInfo() << "Skipping CPU control: device is M900 but not PWC30";
    // }

    m_upmCpuControl->setAllCorePolicyCtl(strategy, "default");
    m_upmCpuControl->setAllCoreMaxFreqLevelCtl(strategy, "default");
    m_upmCpuControl->setAllCoreMinFreqPercentageCtl(strategy, "default");
}

bool SysdbusRegister::FirstRunDonotSuspend()
{
    if (1 == m_powerConfig->getPowerManagerRunTimes()
    &&  true == UpmCustomHardwareConfig::self()->firstRunDonotSuspend()) {
        return true;
    }
    return false;
}

void SysdbusRegister::GpuFrequencyModulation(int strategy)
{
    m_gpuPowerControl->setGpuFrequencyMode(strategy);
}

void SysdbusRegister::SetDevicePowerCtl(int strategy)
{
    m_devPowerControl->SetDevicePowerCtl(strategy, "default");
}

int SysdbusRegister::GetCpuPolicy()
{
    return m_cpuPolicy;
}

void SysdbusRegister::SystemIdleReserveCpu0(bool systemIdle)
{
    m_upmCpuControl->systemIdleReserveCpu0(systemIdle);
}

void SysdbusRegister::EnableReduceCpuFrequency(bool enable)
{
    m_upmCpuControl->enableBalanceModeFrequency(enable);
}

void SysdbusRegister::EnableSystemIdleReserveCpu0(bool enable, int cpuPolicy)
{
    m_upmCpuControl->enableSystemIdleReserveCpu0(enable, cpuPolicy);
}

void SysdbusRegister::EnableAutoSwitchCpuCore(bool enable, int cpuPolicy)
{
    m_upmCpuControl->enableAutoSwitchCpuCore(enable, cpuPolicy);
}

int SysdbusRegister::GetCpuCoreNum()
{
    return m_upmCpuControl->getCpuCoreNum();
}

QString SysdbusRegister::GetCpuCorePolicy(int cpuCoreId)
{
    return m_upmCpuControl->getCpuCorePolicy(cpuCoreId);
}

bool SysdbusRegister::isFlemingXModel()
{
    HardwareReader reader;
    return reader.isFlemingX();
}
