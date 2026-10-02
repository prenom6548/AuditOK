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

#include "idlenesswatcher.h"

IdlenessWatchcer::IdlenessWatchcer() {
    initBrightness();

    m_idleBrightnessPercentage = m_powerManagerGsettings->get(IDLE_BRIGHTNESS).toInt();
    if (m_idleBrightnessPercentage >= 50 && m_idleBrightnessPercentage < 100) {
        m_idleReduceBrightness = true;
    }

    m_idlePowerPolicyAuto = m_powerManagerGsettings->get(IDLE_POWER_POLICY_AUTO).toBool();

    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == IDLE_BRIGHTNESS) {
            m_idleBrightnessPercentage = m_powerManagerGsettings->get(IDLE_BRIGHTNESS).toInt();
            if (m_idleBrightnessPercentage >= 50 && m_idleBrightnessPercentage < 100) {
                m_idleReduceBrightness = true;
            } else {
                m_idleReduceBrightness = false;
            }
        }
        if (key == IDLE_POWER_POLICY_AUTO) {
            m_idlePowerPolicyAuto = m_powerManagerGsettings->get(IDLE_POWER_POLICY_AUTO).toBool();
        }
    });

    isFirstRunDonotSuspend();
}

IdlenessWatchcer::~IdlenessWatchcer()
{
    delete mTurnOffDisplayTimer;
    delete mReduceBrightnessTimer;
    delete mSuspendTimer;
}

void IdlenessWatchcer::initIdleWatcher()
{
    //初始化gsettings,同时读配置文件
    initSettingsConnect();

    initTimer();

    //监听到空闲信号，开始计时，启动timingBegins函数
    QDBusConnection::sessionBus().connect(
        QString(),
        QString(SESSION_MANAGER_PATH),
        GNOME_SESSION_MANAGER,
        "StatusChanged",
        this,
        SLOT(timingBegins(quint32)));

    QDBusConnection::systemBus().connect(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        "org.freedesktop.login1.Manager",
        "PrepareForSleep",
        this,
        SLOT(timingStop()));
    connect(mTurnOffDisplayTimer, &QTimer::timeout, [this] {
        turnOffDisplay();
        turnOffDisplaySignal(true);

        QProcess process;
        process.start("ukui-screensaver-command -b idle");
        //等待命令执行结束
        process.waitForFinished();
        mTurnOffDisplayTimer->stop();
    });
    connect(mReduceBrightnessTimer, &QTimer::timeout, [this] {
        reduceBrightness();
        mReduceBrightnessTimer->stop();
    });
    connect(mSuspendTimer, &QTimer::timeout, [this] {
        if (!isDisablePowerOption() && false == m_firstRunDonotSuspend && isSuspendSupply()) {
            suspend();
        }
        mSuspendTimer->stop();
    });
    //监听到非空闲信号，打断计时
}

void IdlenessWatchcer::initSettingsConnect()
{
    readSettings();
    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=] { readSettings(); });
}

void IdlenessWatchcer::initTimer()
{
    mTurnOffDisplayTimer = new QTimer;
    mReduceBrightnessTimer = new QTimer;
    mSuspendTimer = new QTimer;
}

void IdlenessWatchcer::readSettings()
{
    //读配置文件
    mTurnOffDisplayTimeBat = m_powerManagerGsettings->get(SLEEP_DISPLAY_BAT_KEY).toInt();
    qDebug() << "mTurnOffDisplayTimeBat" << mTurnOffDisplayTimeBat;
    mTurnOffDisplayTimeAc = m_powerManagerGsettings->get(SLEEP_DISPLAY_AC_KEY).toInt();
    qDebug() << "mTurnOffDisplayTimeAc" << mTurnOffDisplayTimeAc;
    mSuspendTimeAc = m_powerManagerGsettings->get(SLEEP_COMPUTER_AC).toInt();
    qDebug() << "mSuspendTime" << mSuspendTimeAc;
    mSuspendTimeBat = m_powerManagerGsettings->get(SLEEP_COMPUTER_BATTERY).toInt();
}

void IdlenessWatchcer::timingBegins(quint32 time)
{
    qDebug() << "Idle signal received from SessionManager" << time;
    if (mSessionVersion == time) {
        startTimer();

        idleReduceBrightness();

        idleSwitchPowerPolicy();

        controlPowerManagement(SYSTEM_IDLE_RESERVE_CPU0, true);
    } else {
        timingStop();

        recoveryBrightness();

        recoveryPowerPolicy();

        controlPowerManagement(SYSTEM_IDLE_RESERVE_CPU0, false);
    }
}

int IdlenessWatchcer::turnOffDisplayTime()
{
    if (true == getBatteryState()) {
        qDebug() << "Time to turn off the display in battery state" << mTurnOffDisplayTimeBat;
        return mTurnOffDisplayTimeBat;
    }
    qDebug() << "Time to turn off the display in power state" << mTurnOffDisplayTimeAc;
    return mTurnOffDisplayTimeAc;
}

int IdlenessWatchcer::suspendTime()
{
    if (true == getBatteryState()) {
        qDebug() << "Time to suspend in battery state" << mSuspendTimeBat;
        return mSuspendTimeBat;
    }
    qDebug() << "Time to suspend in power state" << mSuspendTimeAc;
    return mSuspendTimeAc;
}

void IdlenessWatchcer::isFirstRunDonotSuspend()
{
    QDBusInterface iface(POWER_MANAGEMENT_SERVICE,
                         POWER_MANAGEMENT_PATH,
                         POWER_MANAGEMENT_INTERFACE_NAME,
                         QDBusConnection::systemBus());
    QDBusReply<bool> reply = iface.call(FIRST_RUN_DONOT_SUSPEND);
    if (reply.isValid()) {
        m_firstRunDonotSuspend = reply.value();
    }
    qInfo() << "first run donot suspend :" << m_firstRunDonotSuspend;
}

void IdlenessWatchcer::timingStop()
{
    qDebug() << "timing stop";
    if (mTurnOffDisplayTimer->isActive()) {
        mTurnOffDisplayTimer->stop();
    }
    if (mSuspendTimer->isActive()) {
        mSuspendTimer->stop();
    }
    if (mReduceBrightnessTimer->isActive()) {
        mReduceBrightnessTimer->stop();
    }
}

void IdlenessWatchcer::startTimer()
{
    int mTurnOffDisplayTime = turnOffDisplayTime() - mSessionTime;
    int mSuspendTime = suspendTime() - mSessionTime;
    int mBrightnessTime = -1;

    if (0 <= mTurnOffDisplayTime) {
        mTurnOffDisplayTimer->start(mTurnOffDisplayTime * 1000);
    } else {
        qDebug() << "TurnOffDisplayTimer not started!";
    }
    if (0 < mSuspendTime) {
        mSuspendTimer->start(mSuspendTime * 1000);
    } else {
        qDebug() << "SuspendTimer not started!";
    }

    // 仅在非FlemingX设备上启动降低亮度定时器
    if(!m_isFlemingX) {
        if (mTurnOffDisplayTime > 0) {
            if (mSuspendTime > 0 && mSuspendTime < mTurnOffDisplayTime) {
                mBrightnessTime = mSuspendTime - 10;
            } else {
                mBrightnessTime = mTurnOffDisplayTime - 10;
            }
            mReduceBrightnessTimer->start(mBrightnessTime * 1000);
        } else {
            if (mSuspendTime > 0) {
                mBrightnessTime = mSuspendTime - 10;
                mReduceBrightnessTimer->start(mBrightnessTime * 1000);
            }
        }
        qDebug() << "brightness time:" << mBrightnessTime;
    }
}

void IdlenessWatchcer::reduceBrightness()
{
    if(m_isFlemingX) {
        qDebug() << "flemingx机器，不支持关闭显示器前降低亮度";
        return;
    }

    if (false == m_idleReduceBrightness) {
        getUserBrightness();
        mBrightness = mUserBrightness * 0.7;
    } else {
        mBrightness = mBrightness * 0.7;
    }
    if(mBrightness == 0) {
        qWarning()<<"用户亮度被重置为0，进入睡眠和关闭显示器前10s不再调整亮度";
    } else {
        controlPowerManagement(REGULATE_BRIGHTNESS, mBrightness);
    }

    qDebug() << "Reduce the brightness before turning off the display."
             << "the brightness value is:" << mBrightness;
}

void IdlenessWatchcer::idleReduceBrightness()
{
    if(m_isFlemingX) {
        qDebug() << "flemingx机器，不支持空闲状态降低亮度";
        return;
    }

    if (false == m_idleReduceBrightness) {
        return ;
    }

    // 先检查 mMaxBrightness 是否有效
    if (mMaxBrightness <= 0) {
        qWarning() << "mMaxBrightness 无效，重新初始化";
        initBrightness();
    }

    getUserBrightness();

    if (true == getBatteryState()) {
        mBrightness = mUserBrightness * m_idleBrightnessPercentage/100;
        if(mBrightness == 0) {
            qWarning()<<"用户亮度被重置为0，进入空闲模式不再调整亮度";
        } else {
            controlPowerManagement(REGULATE_BRIGHTNESS, mBrightness);
        }
    } else {
        mBrightness = mUserBrightness;
    }

    qDebug() << "The system enters the idle state."
             << "the brightness value is:" << mBrightness;
}

void IdlenessWatchcer::recoveryBrightness()
{
    if(m_isFlemingX) {
        return;
    }
    
    getUserBrightness();
    if(mUserBrightness == 0) {
        qWarning()<<"用户亮度被重置为0，退出空闲模式不再调整亮度";
    } else {
        controlPowerManagement(REGULATE_BRIGHTNESS, mUserBrightness);
    }
}

void IdlenessWatchcer::idleSwitchPowerPolicy()
{
    int powerPolicy;

    if (true == getBatteryState()) {
        powerPolicy = m_powerManagerGsettings->get(POWER_POLICY_BATTERY).toInt();
    } else {
        powerPolicy = m_powerManagerGsettings->get(POWER_POLICY_AC).toInt();
    }
    if (Balance == powerPolicy) {
        controlPowerManagement(GPU_FREQUENCY_MODULATION, EnergySaving);
    }

    if (true == m_idlePowerPolicyAuto) {
        if (EnergySaving != powerPolicy) {
            controlPowerManagement(CPU_FREQUENCY_MODULATION, EnergySaving);
        }
    }
}

void IdlenessWatchcer::recoveryPowerPolicy()
{
    int powerPolicy;

    if (true == getBatteryState()) {
        powerPolicy = m_powerManagerGsettings->get(POWER_POLICY_BATTERY).toInt();
    } else {
        powerPolicy = m_powerManagerGsettings->get(POWER_POLICY_AC).toInt();
    }
    if (Balance == powerPolicy) {
        controlPowerManagement(GPU_FREQUENCY_MODULATION, powerPolicy);
    }

    if (true == m_idlePowerPolicyAuto) {
        if (EnergySaving != powerPolicy) {
            controlPowerManagement(CPU_FREQUENCY_MODULATION, powerPolicy);
        }
    }
}

bool IdlenessWatchcer::getBatteryState()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    }
    qDebug() << "The battery status obtained by closing the cover is abnormal!";
    return false;
}

bool IdlenessWatchcer::isDisablePowerOption()
{
    int ret;
    QDBusInterface gmInter("org.ukui.SettingsDaemon",
                                  "/GlobalSignal",
                                  "org.ukui.SettingsDaemon.GlobalSignal",
                                  QDBusConnection::sessionBus());
    if(!gmInter.isValid()) {
        qDebug()<<"interface is unvalid";
        return false;
    }

    QDBusReply<bool> boolRep = gmInter.call("isHidePoweroffUi");
    ret = boolRep.value();
    if(ret) {
        qDebug() << "hide power option.........." ;
    }
    return ret;
}

bool IdlenessWatchcer::isSuspendSupply()
{
    QDBusInterface iface("org.freedesktop.login1",
                         "/org/freedesktop/login1",
                         "org.freedesktop.login1.Manager",
                         QDBusConnection::systemBus(),this);
    if(!iface.isValid()){
        qWarning() << "Create login1 CanSuspend Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return false;
    }
    QDBusReply<QString> HibernateInfo;
    HibernateInfo = iface.call("CanSuspend");
    if (HibernateInfo == "yes" || HibernateInfo == "challenge") {
        return true;
    }
    return false;
}
