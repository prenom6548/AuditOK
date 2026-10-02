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

#include "eventwatcher.h"

EventWatcher::EventWatcher()
{
    const QByteArray powerManagerSettingsId(POWER_MANAGER_SETTINGS);
    m_powerManagerGsettings = new QGSettings(powerManagerSettingsId);

    const QByteArray screenSaverSettingsId(SCREEN_SAVER_SETTINGS);
    if (QGSettings::isSchemaInstalled(screenSaverSettingsId)) {
        m_screenSaverGsettings = new QGSettings(screenSaverSettingsId);
    } else {
        qWarning() << "ScreenSaver GSettings schema not installed:" << screenSaverSettingsId;
    }
    
    const QByteArray autoBrightnessSettingsId(AUTO_BRIGHTNESS_SETTINGS);
    if(QGSettings::isSchemaInstalled(autoBrightnessSettingsId)){
        m_autoBrightnessGsettings = new QGSettings(autoBrightnessSettingsId);
    } else {
        qWarning() << "schema" << autoBrightnessSettingsId << "is not installed";
    }


    m_globalConfig = new GlobalConfig();
    connect(m_globalConfig, SIGNAL(userActiveSignal(bool)), this, SLOT(userActiveSlot(bool)));
    m_pInitTimer = new QTimer(this);
    connect(m_pInitTimer, SIGNAL(timeout()), this, SLOT(onInitTimerOut()));
    
    m_isFlemingX = checkDeviceModel("isFlemingXModel");
    m_reserveBrightnessPercent = m_isFlemingX ? 0.0 : 0.1;
}

EventWatcher::~EventWatcher()
{
    delete m_powerManagerGsettings;
    delete m_screenSaverGsettings;
    if(m_autoBrightnessGsettings != nullptr){
        delete m_autoBrightnessGsettings;
        m_autoBrightnessGsettings = nullptr;
    }
    delete m_globalConfig;
    delete m_pInitTimer;
}

void EventWatcher::initEventWatcher()
{
    m_pInitTimer->start(1200);
    double userBrightnessPercent = 0.0f;

    initBrightness();

    if (-1.0 == m_powerManagerGsettings->get(BRIGHTNESS_AC).toDouble()) {
        if (mMaxBrightness > 0) {
            userBrightnessPercent = m_isFlemingX ? m_flx->convert2UiValue(mUserBrightness) : (mUserBrightness - mReserveBrightness) * 100 / mDynamicBrightness;
            if (userBrightnessPercent <= 20.0) {
                userBrightnessPercent = DEFAULT_BRIGHTNESS_PERCENT; //默认亮度为70%
                qWarning() << "user brightness is under 20.0, set default brightness percent:" << userBrightnessPercent;
            }
        } 
        m_powerManagerGsettings->set(BRIGHTNESS_AC, userBrightnessPercent);
        qDebug() << "init user brightness ac is :" << userBrightnessPercent;
    }
    getUserBrightness();
    setBrightness();

    if (true == m_powerManagerGsettings->get(PERCENT_LOW_REDUCE_BRIGHTNESS).toBool()) {
        percentageLowRegulateBrightness(false);
    }

    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == BRIGHTNESS_AC) {
            getUserBrightness();
            setBrightness();
            m_powerManagerGsettings->set(PERCENT_LOW_REDUCE_BRIGHTNESS, false);
        }
    });

    initAcpiSocket();
}

void EventWatcher::initAcpiSocket()
{
    m_socket = new QLocalSocket(this);
    connect(m_socket, &QLocalSocket::connected, this, [=]() { qDebug() << "acpi socket connected success"; });
    connect(m_socket, &QLocalSocket::readyRead, this, [=]() {
        QByteArray data = m_socket->readAll();
        QString str = QString(data).simplified();
        if (-1 != str.indexOf("thermal_zone LNXTHERM:00 000000f1 00000001", 0, Qt::CaseInsensitive)) {
            hibernate();
        }
    });

    m_socket->connectToServer("/var/run/acpid.socket");
    qDebug() << m_socket->state();
}

void EventWatcher::initBrightness()
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    QDBusReply<bool> reply = iface.call(IS_BRIGHTNESS_ENHANCEMENT_SUPPORTED);
    if (reply.isValid()) {
        m_isBrightnessEnhancementSupported = reply.value();
    } else {
        qWarning() << "get " << IS_BRIGHTNESS_ENHANCEMENT_SUPPORTED << " fail";
    }

    QString isSupportBrightnessEnhancement = m_isBrightnessEnhancementSupported ? "true" : "false";
    qInfo() << "Is support brightness  enhancement " << isSupportBrightnessEnhancement;

    mMaxBrightness = controlPowerManagement(GET_MAX_BRIGHTNESS);
    qDebug() << "max brightness is :" << mMaxBrightness;

    m_brightness350Nit = (int)(mMaxBrightness * BRIGHTNESS_ENHANCEMENT_THRESHOLD_PERCENT);

    mUserBrightness = controlPowerManagement(GET_BRIGHTNESS);
    qDebug() << "the current brightness value of the machine is :" << mUserBrightness;

    /**
      未开启亮度增强，但是亮度值超过350nit对应的亮度，则设置为350nit对应的亮度
      **/
    if ((mUserBrightness > m_brightness350Nit)
            && m_isBrightnessEnhancementSupported
            && !m_powerManagerGsettings->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toBool())
    {
        qWarning() << "not enhancement but current brightness value over 350nit value:" << m_brightness350Nit;
        mUserBrightness = m_brightness350Nit;
    }

    // 如果支持亮度增强，百分百亮度对应350nit的亮度值；150%的亮度对应最大的亮度值；注意此时最小亮度值和动态变化亮度值已经是判断过亮度增强了
    if (mMaxBrightness > 0 ) {
        mReserveBrightness = (m_isBrightnessEnhancementSupported ? m_brightness350Nit : mMaxBrightness) * m_reserveBrightnessPercent;
        mDynamicBrightness = (m_isBrightnessEnhancementSupported ? m_brightness350Nit : mMaxBrightness) - mReserveBrightness;
        qInfo() << "max brightness is " << mMaxBrightness << "  reserve brightness is " << mReserveBrightness << "  dynamicbbrightness is " << mDynamicBrightness;
    } else {
        qWarning() << "max brightness is invalued : " << mMaxBrightness;
        mReserveBrightness = 0;
        mDynamicBrightness = 0;
    }

    m_flx = new FlemingX(mMaxBrightness, this);
    qDebug() << "init reserve brightness is :" << mReserveBrightness;
}

void EventWatcher::getUserBrightness()
{
    double userBrightnessPercent = m_powerManagerGsettings->get(BRIGHTNESS_AC).toDouble();
    qDebug() << "user brightness percent is :" << userBrightnessPercent;

    if (userBrightnessPercent < 0) {
        qWarning() << "invalued user brightness percent " << userBrightnessPercent;
        return;
    }

    if (m_isBrightnessEnhancementSupported) {//支持亮度增强
        if (m_powerManagerGsettings->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toBool()) { //且开启了开关
            userBrightnessPercent = qMin(userBrightnessPercent, 150.0);
        } else {
            userBrightnessPercent = qMin(userBrightnessPercent, 100.0);
        }

        if (userBrightnessPercent > 100.0) {
            mUserBrightness = m_brightness350Nit
                    + (mMaxBrightness - m_brightness350Nit) * (userBrightnessPercent - 100.0)/50;
        } else {
            mUserBrightness = (userBrightnessPercent <= 20 ? (userBrightnessPercent * 0.5 + m_reserveBrightnessPercent * 100) :
                                                             userBrightnessPercent)*m_brightness350Nit/100;
        }
        qInfo() << "support brightness  enhancement user brightness percent " << userBrightnessPercent << "  user brightness " << mUserBrightness;
    } else {//不支持亮度增强
        userBrightnessPercent = qMin(userBrightnessPercent , 100.0);//最大值为20，则20%应该对应4，10%对应3,25%:对应5，80%对应16。
                                                                    //最大值为200，则20%应该对应40，10%对应30,25%:50，80%对应160。
                                                                    //最大值为255，则20%应该对应50.5，10%对应37.75,25%:63.75，80%对应20。
        if(m_isFlemingX){
            mUserBrightness = m_flx->convert2NodeValue(userBrightnessPercent);
        } else {
            mUserBrightness = (userBrightnessPercent <= 20 ? (userBrightnessPercent * 0.5 + m_reserveBrightnessPercent * 100) : userBrightnessPercent)*mMaxBrightness/100;
        }
    }

    qInfo() << "user brightness is :" << mUserBrightness;
}

void EventWatcher::setBrightness()
{
    controlPowerManagement(REGULATE_BRIGHTNESS, mUserBrightness);
}

void EventWatcher::setGsettings(QString key, QVariant value)
{
    m_powerManagerGsettings->set(key, value);
}

void EventWatcher::turnOffDisplay()
{
    //调用关闭显示器的接口
    if ("wayland" == qgetenv("XDG_SESSION_TYPE")) {
        system("export QT_QPA_PLATFORM=wayland && kscreen-doctor --dpms off &");
    } else {
        system("xset dpms force off &");
    }
    qDebug() << "The display is turned off" << qgetenv("XDG_SESSION_TYPE");
}

void EventWatcher::turnOnDisplay()
{
    if ("wayland" == qgetenv("XDG_SESSION_TYPE")) {
        system("export QT_QPA_PLATFORM=wayland && kscreen-doctor --dpms on &");
    } else {
        system("xset dpms force on &");
    }
    qDebug() << "The display is turned on" << qgetenv("XDG_SESSION_TYPE");
}

void EventWatcher::suspend()
{
    controlLogin1Manager(LOGIN1_DBUS_SET_SUSPEND);
}

void EventWatcher::hibernate()
{
    controlLogin1Manager(LOGIN1_DBUS_SET_HIBERNATE);
}

void EventWatcher::powerOff()
{
    controlLogin1Manager(LOGIN1_DBUS_SET_POWEROFF);
}

void EventWatcher::turnOffDisplaySignal(bool state)
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "ukui.power.manager", "TurnOffDisplay");
    msg << state;
    QDBusConnection::sessionBus().send(msg);
    qDebug() << "send turn off display msg, state:" << state;
}

void EventWatcher::percentageLowRegulateBrightness(bool percentageLowState)
{
    if (true == m_isFlemingX) {
        qDebug() << "flemingx机器，不支持低电量下降低亮度";
        return;
    }
    
    m_powerManagerGsettings->blockSignals(true);

    double userBrightnessPercent = m_powerManagerGsettings->get(BRIGHTNESS_AC).toDouble();
    if (true == percentageLowState) {
        userBrightnessPercent =  userBrightnessPercent > 10 ? userBrightnessPercent - 10 : 0;
    } else {
        if (true == m_powerManagerGsettings->get(PERCENT_LOW_REDUCE_BRIGHTNESS).toBool()) {
            userBrightnessPercent += 10;
        } else {
            m_powerManagerGsettings->blockSignals(false);
            return ;
        }
    }

    userBrightnessPercent = qMin(userBrightnessPercent, m_isBrightnessEnhancementSupported ? 150.0 : 100.0);

    if (userBrightnessPercent <= 100) {//即使支持亮度增强模式，百分比小于100时直接计算即可，因为mReserveBrightness、mDynamicBrightness均计算过亮度增强因素
        mUserBrightness = (userBrightnessPercent <= 20 ? (userBrightnessPercent * 0.5 + m_reserveBrightnessPercent * 100) : userBrightnessPercent)
                * (mReserveBrightness + mDynamicBrightness) / 100;
    } else {
        if (m_powerManagerGsettings->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toBool()) {
            // 百分比大于100且开启了增强模式开关
            mUserBrightness = m_brightness350Nit
                    + (mMaxBrightness - m_brightness350Nit) * (userBrightnessPercent - 100.0)/50;
        } else {
             mUserBrightness = m_brightness350Nit;
        }
    }

    controlPowerManagement(REGULATE_BRIGHTNESS, mUserBrightness);

    m_powerManagerGsettings->set(BRIGHTNESS_AC, userBrightnessPercent);

    m_powerManagerGsettings->set(PERCENT_LOW_REDUCE_BRIGHTNESS, percentageLowState);

    QTimer::singleShot(10, this, [=]() {
        m_powerManagerGsettings->blockSignals(false);
    });
}

void EventWatcher::handleMiniBrightness()
{
    setBrightness();
}

void EventWatcher::controlLogin1Manager(QString action)
{
    QDBusInterface dbusInterface("org.freedesktop.login1",
                                 "/org/freedesktop/login1",
                                 "org.freedesktop.login1.Manager",
                                 QDBusConnection::systemBus());
    dbusInterface.call(action, false);
    qInfo() << "system will" << action;
}

bool EventWatcher::checkDeviceModel(const QString &methodName)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, 
        POWER_MANAGEMENT_PATH, 
        POWER_MANAGEMENT_INTERFACE_NAME, 
        QDBusConnection::systemBus());

    // 检查接口是否有效
    if (!iface.isValid()) {
        QString errorMsg = QDBusConnection::systemBus().lastError().message();
        qWarning() << "设备检测错误 - DBus接口无效:" << errorMsg;
        return false;
    }

    // 调用DBus方法
    QDBusReply<bool> reply = iface.call(methodName);
    
    // 检查回复是否有效
    if (!reply.isValid()) {
        qWarning() << "设备检测失败 - DBus调用错误:" << reply.error().name() 
                   << "-" << reply.error().message();
        return false;
    }
    
    return reply.value();
}

//此处代码后期可以进行优化
int EventWatcher::controlPowerManagement(const QString &type)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    QDBusReply<qulonglong> reply = iface.call(type);
    return reply.value();
}

void EventWatcher::controlPowerManagement(const QString &type, const qulonglong &brightness)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    iface.call(type, brightness);
}

void EventWatcher::controlPowerManagement(const QString &type, const int &value)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    iface.call(type, value);
}

void EventWatcher::controlPowerManagement(const QString &type, const bool &value)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    iface.call(type, value);
}

void EventWatcher::controlPowerManagement(const QString &type, const QString &contrl)
{
    QDBusInterface iface(
        POWER_MANAGEMENT_SERVICE, POWER_MANAGEMENT_PATH, POWER_MANAGEMENT_INTERFACE_NAME, QDBusConnection::systemBus());
    iface.call(type, contrl);
}


void  EventWatcher::userActiveSlot(bool isLogin)
{
    if(isLogin)
    {
        syncFromGlobalConfig();
        qDebug() <<"user login syncFromGlobalConfig";
        emit allUserSyncFinished();
    }
}

void EventWatcher::onInitTimerOut()
{
    static int retry = 0;
    double userBrightnessPercent = 0.0f;
    initBrightness();

    if (0 == mMaxBrightness && retry++ < 3) {
       qDebug() << "mMaxBrightness get error!";
       return;
    }
    m_pInitTimer->stop();
    qDebug() << "mMaxBrightness:"<< mMaxBrightness << retry;
    m_globalConfig->initGlobalConfig();
    userActiveSlot(true);

    qDebug() << "init EventWatcher";

    // 初始化时根据当前电源状态设置睡眠转休眠时间
    handlePowerStateChange(readOnBattery());

    if (-1.0 == m_powerManagerGsettings->get(BRIGHTNESS_AC).toDouble()) {
        if (mDynamicBrightness > 0 && mUserBrightness > mReserveBrightness) {
            if ((100 * mUserBrightness / (mReserveBrightness + mDynamicBrightness)) > 20) {
                userBrightnessPercent = 100 * mUserBrightness / (mReserveBrightness + mDynamicBrightness);
            } else {
                userBrightnessPercent = 2 * 100 * mUserBrightness / (mReserveBrightness + mDynamicBrightness) - m_reserveBrightnessPercent * 100;
            }
        } else {
            qWarning() << "currnt brightness " << mUserBrightness << "is below ReserveBrightness " << mReserveBrightness << " or Dynamic Brightness invalued " << mDynamicBrightness;
            userBrightnessPercent = 0.0;
        }
        m_powerManagerGsettings->set(BRIGHTNESS_AC, userBrightnessPercent);
        qDebug() << "init user brightness ac is :" << userBrightnessPercent;

    }

    // 记录当前系统实际亮度
    qulonglong currentKernelBrightness = mUserBrightness;

    // 根据 gsettings 计算目标亮度，结果写回 mUserBrightness
    getUserBrightness();

    if (mUserBrightness != currentKernelBrightness) {
        qInfo() << "Brightness mismatch between gsettings and system, adjust:"
                << "kernel =" << currentKernelBrightness
                << "gsettings =" << mUserBrightness;
        setBrightness();  // 下发调节
    } else {
        qInfo() << "Brightness from gsettings matches system, skip adjust:"
                << "value =" << currentKernelBrightness;
    }

    m_globalConfig->writeGlobalParam(BRIGHTNESS_AC, m_powerManagerGsettings->get(BRIGHTNESS_AC));
    
    if (true == m_powerManagerGsettings->get(PERCENT_LOW_REDUCE_BRIGHTNESS).toBool()) {
        percentageLowRegulateBrightness(false);
    }

    connect(m_powerManagerGsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == BRIGHTNESS_AC) {
            getUserBrightness();
            setBrightness();
            m_powerManagerGsettings->set(PERCENT_LOW_REDUCE_BRIGHTNESS, false);
        }

        if (key == BRIGHTNESS_ENHANCEMENT_SWITCH)
        {
            qInfo() << "brightness enhancement switch is " << (m_powerManagerGsettings->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toBool() ? "open" : "close");
            if (!m_powerManagerGsettings->get(BRIGHTNESS_ENHANCEMENT_SWITCH).toBool()
                    && (m_powerManagerGsettings->get(BRIGHTNESS_AC) > 100)) {
                qWarning() << "now brightness over 100% reset 100%";
                m_powerManagerGsettings->set(BRIGHTNESS_AC, 100);//由connect信号处理亮度百分比变化
            }
        }
        
        // 处理睡眠转休眠时间设置变化
        if (key == HIBERNATE_COMPUTER_AC_KEY) {
            // AC设置变化时，如果当前使用AC，则更新设置
            if (!readOnBattery()) {
                handlePowerStateChange(false);
            }
        } else if (key == HIBERNATE_COMPUTER_BATT_KEY) {
            // 电池设置变化时，如果当前使用电池，则更新设置
            if (readOnBattery()) {
                handlePowerStateChange(true);
            }
        }
        
        m_globalConfig->writeGlobalParam(key, m_powerManagerGsettings->get(key));
    });

    if (m_screenSaverGsettings) {
        connect(m_screenSaverGsettings, &QGSettings::changed, this, [=](const QString &key) {
            qDebug() << "write screen saver settings";
            if (key == SLEEPACTIVATIONENABLED || key == CLOSEACTIVATIONENABLED) {
                m_globalConfig->writeGlobalParam(key, m_screenSaverGsettings->get(key));
                qDebug() << "write screen saver settings";
            }
        });
    } else {
        qWarning() << "m_screenSaverGsettings is null, skip screen saver gsettings connect";
    }

    initAcpiSocket();
}

void EventWatcher::syncFromGlobalConfig()
{
    QVariantMap map;
    bool isAutoBrightnessEnabled = false;
    if(m_autoBrightnessGsettings != nullptr){
        isAutoBrightnessEnabled = m_autoBrightnessGsettings->get(AUTO_BRIGHTNESS_KEY).toBool();
    } else {
        qWarning() << "Auto brightness QGSettings is null";
    }

    bool ret = m_globalConfig->readAllGlobalParam(map);
    if(ret == true){
        Q_FOREACH(const QString key, map.keys()) {
            qDebug()<<"syncFromGlobalConfig set:"<<key<<" value: "<<map[key];

            if (isAutoBrightnessEnabled && key == BRIGHTNESS_AC) {
                qInfo() << "Auto brightness enabled, skipping brightness sync for:" << key;
                continue;
            }
            // 显示剩余时间不做多用户同步，直接跳过
            if(key == DISPLAY_LEFT_TIME){
                qInfo() << "skip syncFromGlobalConfig, deprecated key:" << key;
                continue;
            }

            // 屏保相关 key
            if (key == SLEEPACTIVATIONENABLED || key == CLOSEACTIVATIONENABLED) {
                m_screenSaverGsettings->set(key,map[key]);
                continue;
            }

            // 电源管理 schema 中存在的 key
            if (m_powerManagerGsettings->keys().contains(key)) {
                m_powerManagerGsettings->set(key, map[key]);
            } else {
                qInfo() << "skip syncFromGlobalConfig, unknown key:" << key;
            }
         }
    }
}

void EventWatcher::setSuspend2HibernateTime(int seconds)
{
    QDBusInterface dbusInterface("org.freedesktop.login1",
                                "/org/freedesktop/login1",
                                "org.freedesktop.login1.Manager",
                                QDBusConnection::systemBus());

    if (!dbusInterface.isValid()) {
        qWarning() << "Failed to create DBus interface for login1 Manager:"
                  << QDBusConnection::systemBus().lastError().message();
        return;
    }
    // -1 表示从不，其他值为秒数，需转换为分钟
    int minutes = seconds == -1 ? seconds : seconds / 60;
    QDBusReply<void> reply = dbusInterface.call("Suspend2HibernateTime", minutes);

    if (!reply.isValid()) {
        qWarning() << "Failed to set Suspend2HibernateTime:" << reply.error().message();
    } else {
        qInfo() << "Successfully set Suspend2HibernateTime to" << minutes << "minutes (" << seconds <<  "seconds)";
    }
}

bool EventWatcher::readOnBattery()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, FREEDESKTOP_UPOWER, QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", "org.freedesktop.UPower", "OnBattery");
    if (reply.isValid()) {
        return reply.value().toBool();
    }
    qWarning() << "Failed to read power state, defaulting to AC";
    return false;
}

void EventWatcher::handlePowerStateChange(bool onBattery)
{
    // 根据电源状态选择对应的key
    const char* key = onBattery ? HIBERNATE_COMPUTER_BATT_KEY : HIBERNATE_COMPUTER_AC_KEY;
    const char* powerType = onBattery ? "battery" : "AC";
    
    if (m_powerManagerGsettings->keys().contains(key)) {
        int hibernateTimeSeconds = m_powerManagerGsettings->get(key).toInt();
        setSuspend2HibernateTime(hibernateTimeSeconds);
        qDebug() << "Power state changed to" << powerType << ", set Suspend2HibernateTime to" << hibernateTimeSeconds << "seconds";
    }
}
