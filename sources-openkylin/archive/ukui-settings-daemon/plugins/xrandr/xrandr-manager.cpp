/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2012 by Alejandro Fiestas Olivares <afiestas@kde.org>
 * Copyright 2016 by Sebastian Kügler <sebas@kde.org>
 * Copyright (c) 2018 Kai Uwe Broulik <kde@broulik.de>
 *                    Work sponsored by the LiMux project of
 *                    the city of Munich.
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include <QX11Info>
#include <QtXml>
#include <QtConcurrent/QtConcurrent>
#include <cmath>
#include "xrandr-manager.h"
#include "touch-calibrate.h"
#include <QOrientationReading>
#include <memory>
#include <QDBusMessage>

extern "C"{
#include <X11/extensions/Xrandr.h>
#include "clib-syslog.h"
}

#define SETTINGS_XRANDR_SCHEMAS     "org.ukui.SettingsDaemon.plugins.xrandr"
#define XRANDR_ROTATION_KEY         "xrandr-rotations"


XrandrManager::XrandrManager():
    m_outputsChangedSignal(eScreenSignal::isNone),
    m_acitveTimer(new QTimer(this)),
    m_outputsInitTimer(new QTimer(this)),
    m_offUsbScreenTimer(new QTimer(this)),
    m_onUsbScreenTimer(new QTimer(this))
{

    m_xrandrDbus = new xrandrDbus(this);
    m_xrandrSettings = new QGSettings(SETTINGS_XRANDR_SCHEMAS);
    m_pXsettingsSettings = new QGSettings(GSETTINGS_XSETTINGS_SCHEMA);
//    m_xrandrAdaptor = new XrandrAdaptor(m_xrandrDbus);
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(DBUS_XRANDR_NAME)) {
        sessionBus.registerObject(DBUS_XRANDR_PATH,
                                  m_xrandrDbus,
                                  QDBusConnection::ExportAllContents);
    } else {
        USD_LOG(LOG_ERR, "register dbus error");
    }

    m_outputModeEnum = QMetaEnum::fromType<UsdBaseClass::eScreenMode>();

    m_statusManagerDbus = new QDBusInterface(DBUS_STATUSMANAGER_NAME,
                                             DBUS_STATUSMANAGER_PATH,
                                             DBUS_STATUSMANAGER_INTERFACE,
                                             QDBusConnection::sessionBus(),
                                             this);

    if (m_statusManagerDbus->isValid()) {
        connect(m_statusManagerDbus, SIGNAL(mode_change_signal(bool)),this,SLOT(doTabletModeChanged(bool)));
        connect(m_statusManagerDbus, SIGNAL(rotations_change_signal(QString)),this,SLOT(doRotationChanged(QString)));
    } else {
        USD_LOG(LOG_ERR, "m_statusManagerDbus error");
    }

    m_onUsbScreenTimer->setSingleShot(true);
    m_offUsbScreenTimer->setSingleShot(true);

    connect(m_offUsbScreenTimer, &QTimer::timeout, this, [=](){
        std::unique_ptr<xrandrConfig> MonitoredConfig = m_outputsConfig->readFile(false);

        if (MonitoredConfig == nullptr ) {
            USD_LOG(LOG_DEBUG,"config a error");
            setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::cloneScreenMode));
            return;
        }
        m_outputsConfig = std::move(MonitoredConfig);
        applyConfig();
    });

    connect(m_onUsbScreenTimer, &QTimer::timeout, this, [=](){
         setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::extendScreenMode));
    });

    connect(m_xrandrDbus,&xrandrDbus::controlScreen,this,&XrandrManager::doCalibrate);
}

void XrandrManager::getInitialConfig()
{
    if (!m_isInitFinish) {
        int count = UsdBaseClass::getConnectOutputCount();
        USD_LOG(LOG_DEBUG,"start 1500 timer...");
        m_outputsInitTimer->start(1500);
        if (count < 1) {
            USD_LOG(LOG_DEBUG,"screens count == %d",count);
            return;
        }

        m_outputsInitTimer->start(1500);
    }

    connect(new KScreen::GetConfigOperation, &KScreen::GetConfigOperation::finished,
            this, [this](KScreen::ConfigOperation* op) {//此处会有malloc_consolidate():invlid chunk size
        if (op->hasError()) {
            USD_LOG(LOG_DEBUG,"Error getting initial configuration：%s",op->errorString().toLatin1().data());
            return;
        }

        USD_LOG(LOG_DEBUG,"stop 1500 timer...");
        m_outputsInitTimer->stop();

        if (m_outputsConfig) {
            if (m_outputsConfig->data()) {
                KScreen::ConfigMonitor::instance()->removeConfig(m_outputsConfig->data());
                for (const KScreen::OutputPtr &output : m_outputsConfig->data()->outputs()) {
                    output->disconnect(this);
                }
                m_outputsConfig->data()->disconnect(this);
            }
            m_outputsConfig = nullptr;
        }

        m_outputsConfig = std::unique_ptr<xrandrConfig>(new xrandrConfig(qobject_cast<KScreen::GetConfigOperation*>(op)->config()));
        m_outputsConfig->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
        m_isInitFinish = true;
        if (initAllOutputs()>1) {
            m_xrandrDbus->mScreenMode = discernScreenMode();
            m_outputsConfig->setScreenMode(m_outputModeEnum.valueToKey(m_xrandrDbus->mScreenMode));
        } else {
            sendOutputsModeToDbus();
        }
        setMaxScaleToXsettings();
        doOutputsConfigurationChanged();
    });

}

XrandrManager::~XrandrManager()
{
}

bool XrandrManager::start()
{
    USD_LOG(LOG_DEBUG,"Xrandr Manager Start");
    connect(m_acitveTimer, &QTimer::timeout, this,&XrandrManager::active);
    m_acitveTimer->start(0);
    return true;
}

void XrandrManager::stop()
{
    USD_LOG(LOG_DEBUG,"Xrandr Manager Stop");

    delete m_acitveTimer;
    m_acitveTimer = nullptr;

    delete m_outputsInitTimer;
    m_outputsInitTimer = nullptr;

    delete m_offUsbScreenTimer;
    m_offUsbScreenTimer = nullptr;

    delete m_onUsbScreenTimer;
    m_onUsbScreenTimer = nullptr;

    delete m_xrandrDbus;
    m_xrandrDbus = nullptr;

    if (m_xrandrSettings) {
        delete m_xrandrSettings;
        m_xrandrSettings = nullptr;
    }

    if(m_statusManagerDbus) {
        delete m_statusManagerDbus;
        m_statusManagerDbus = nullptr;
    }

    if (m_screenSignalTimer) {
        delete m_screenSignalTimer;
        m_screenSignalTimer = nullptr;
    }

    USD_LOG(LOG_DEBUG,"Xrandr Manager Stop");
    return;
}


QSize XrandrManager::getScreenSizeWithXlib(QString outputName, int &width, int &height)
{
    QSize size(0,0);
    if (UsdBaseClass::isWaylandWithKscreen()) {
        return size;
    }

    XRRScreenResources *m_pScreenRes = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (QX11Info::display(), m_pScreenRes, outputId);
        if (outputInfo->connection != RR_Connected) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }

        QString outputname = QString::fromLatin1(outputInfo->name);
        width = outputInfo->mm_width;
        height = outputInfo->mm_height;
        size = QSize(width, height);
        XRRFreeOutputInfo(outputInfo);

        if(!outputName.compare(outputname,Qt::CaseInsensitive)) {
            break;
        }
    }
    XRRFreeScreenResources(m_pScreenRes);
    return size;
}

double XrandrManager::getPreferredScale(const KScreen::OutputPtr  output)
{//只是针对于外接显示器基于当前缩放率进行计算，与设备相关的缩放放在save-param内,显示器的推荐缩放率在其他位置。

    int width = output->sizeMm().width();
    int height = output->sizeMm().height();
    if (width == 0x00 || height == 0x00) {
        getScreenSizeWithXlib(output->name(), width, height);
    }
    return UsdBaseClass::getPreferredScale(width, height,
                                           output->preferredMode()->size().width(), output->preferredMode()->size().height());
}

/*
 * TODO:如果100%缩放率下，有些屏幕会需要150的缩放率，1920x1080这个时候会匹配到1280x720.这个情况不应该出现，沿用1920x1080
 * 但不能影响计算150%的缩放率。
*/
QString XrandrManager::getOuputModIdWithScale(const KScreen::OutputPtr  output)
{
    double scale;
    double outputRefresh = 0;

    QSize setSize(0, 0);
    QSize diffValue(9999,9999);
    QSize prefferedSize;

    QString modeId;

    prefferedSize = output->preferredMode()->size();
    scale = getPreferredScale(output);
    if (scale < 1  || prefferedSize.width() <= 1920 || prefferedSize.height() <= 1080) {//1920x1080这个时候会匹配到1280x720.这个情况不应该出现，沿用1920x1080
        return output->preferredModeId();
    }
    setSize = prefferedSize / scale;

    if (setSize.width() <= 1700 || setSize.height() <= 900) {//限制调优后的分辨率，最小为1700x1080.
         return output->preferredModeId();
    }

    modeId = output->preferredModeId();//优先使用推荐模式，
    USD_LOG_SHOW_PARAM2(setSize.width(), setSize.height());
    USD_LOG_SHOW_PARAM2(prefferedSize.width(), prefferedSize.height());

    Q_FOREACH(auto mode, output->modes()) {
        QSize tempSize;
        double dTempWidth;
        double dSetWidth;

        QSizeF modeSize = mode->size();
        QSizeF prefferSize = prefferedSize;
        if (modeSize.width() / modeSize.height() !=
                prefferSize.width() / prefferSize.height()) {
            USD_LOG(LOG_DEBUG,"skip %s", mode->id().toLatin1().data());
            continue;
        }

        tempSize = QSize(qAbs(mode->size().width() - setSize.width()),
                         qAbs(mode->size().height() - setSize.height()));

        dTempWidth = qAbs(tempSize.width());
        dSetWidth = setSize.width();

        if (dTempWidth / dSetWidth > 0.3) {//通过最佳缩放率计算出来的分辨率，只接受兼容30%以内的差值。无限制画面分辨率只会更小
            USD_LOG(LOG_DEBUG,"skip %s cuz diff so big %f", mode->id().toLatin1().data(), dTempWidth / dSetWidth);
            continue;
        } else {
            USD_LOG(LOG_DEBUG,"ready %s cuz diff %f", mode->id().toLatin1().data(), dTempWidth / dSetWidth);
            if ((tempSize.height() + tempSize.width()) < (diffValue.height() + diffValue.width())) {
                diffValue = tempSize;
                modeId = mode->id();
                outputRefresh = mode->refreshRate();
            } else if ((tempSize.height() + tempSize.width()) == (diffValue.height() + diffValue.width())) {
                if (mode->refreshRate() > outputRefresh) {
                    diffValue = tempSize;
                    modeId = mode->id();
                    outputRefresh = mode->refreshRate();
                } else {
                    USD_LOG(LOG_DEBUG,"skip %s cuz refresh %f small than %f", mode->id().toLatin1().data(), output->preferredMode()->refreshRate(),
                            outputRefresh);
                }
            }
        }
    }
    USD_LOG(LOG_DEBUG,"find id :%s..to",modeId.toLatin1().data());
    return modeId;
}

/*监听旋转键值回调 并设置旋转角度*/
void XrandrManager::doRotationChanged(const QString &rotation)
{
    int value = 0;
    QString angle_Value = rotation;

    if (angle_Value == "normal") {
        value = 1;
    } else if (angle_Value == "left") {
        value = 2;
    } else if (angle_Value == "upside-down") {
        value = 4;
    } else if  (angle_Value == "right") {
        value = 8;
    } else {
        USD_LOG(LOG_ERR,"Find a error !!!");
        return ;
    }

    const KScreen::OutputList outputs = m_outputsConfig->data()->outputs();
    for(auto output : outputs){
        if (!output->isConnected() || !output->isEnabled() || !output->currentMode()) {
            continue;
        }
        output->setRotation(static_cast<KScreen::Output::Rotation>(value));
        USD_LOG(LOG_DEBUG,"set %s rotaion:%s", output->name().toLatin1().data(), rotation.toLatin1().data());
    }
    applyConfig();
}


void XrandrManager::doOutputsConfigurationChanged()
{
    KScreen::GetConfigOperation *gop = new KScreen::GetConfigOperation;
    connect(gop, &KScreen::GetConfigOperation::finished,
            this, [this, gop](KScreen::ConfigOperation* op) {
        bool hadFindPrimary = false;
        m_outputsConfigForOther = std::unique_ptr<xrandrConfig>(new xrandrConfig(qobject_cast<KScreen::GetConfigOperation*>(op)->config()));
        m_outputsConfigForOther->setValidityFlags(KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen);
        KScreen::OutputPtr primary = nullptr;

        Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfigForOther->data()->outputs()) {
            if (output->isPrimary()) {
                primary = output;
                hadFindPrimary = true;
                break;
            }
        }

        if (primary == nullptr) {
            primary = m_outputsConfigForOther->data()->outputs()[0];
        }

        if (primary != nullptr) {
            USD_LOG_SHOW_OUTPUT(primary);
            int width = primary->size().width();
            int height = primary->size().height();

            if (/*!UsdBaseClass::isWaylandWithKscreen() && */(KScreen::Output::Rotation::Left == primary->rotation() ||
                                                              KScreen::Output::Rotation::Right == primary->rotation())) {
                USD_LOG(LOG_DEBUG,"%dx%d",width,height);
                width ^= height ^= width ^= height;
                USD_LOG(LOG_DEBUG,"%dx%d",width,height);
            }
            Q_EMIT m_xrandrDbus->primaryChanged(primary->geometry().x(), primary->geometry().y(),
                                                primary->geometry().width(),
                                                primary->geometry().height(),primary->rotation());
        }

        sendOutputsModeToDbus();
        m_xrandrDbus->sendScreenStateChangedSignal("all", 0);
        gop->deleteLater();
    });
    return;

}

void XrandrManager::calibrateTouchDevice()
{
    if (!m_isInitFinish) {
        return;
    }
    static KScreen::ConfigPtr oldOutputs = nullptr;
    bool needSetMapTouchDevice = false;

    if (oldOutputs != nullptr) {
        Q_FOREACH(const KScreen::OutputPtr &oldOutput, oldOutputs->outputs()) {
            if (needSetMapTouchDevice) {
                break;
            }
            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if(output->isConnected() && oldOutput->name() == output->name()) {
                    if(oldOutput->currentModeId() != output->currentModeId() || oldOutput->pos() != output->pos()
                            || oldOutput->scale() != output->scale() || oldOutput->rotation() != output->rotation() ||
                            oldOutput->isPrimary() != output->isPrimary() || oldOutput->isEnabled() != output->isEnabled() ||
                            oldOutput->isConnected() != output->isConnected()) {
                        needSetMapTouchDevice = true;
                    }
                    if(oldOutput->currentMode().isNull() && output->currentMode().isNull()) {
                        continue;
                    }
                    if(oldOutput->currentMode().isNull() || output->currentMode().isNull()) {
                        needSetMapTouchDevice = true;
                        break;
                    } else {
                        if(oldOutput->currentMode()->size() != output->currentMode()->size()) {
                            needSetMapTouchDevice = true;
                        }
                    }
                }
                if(oldOutput->currentMode().isNull() && output->currentMode().isNull()) {
                    break;
                }
                if(oldOutput->currentMode().isNull() || output->currentMode().isNull()) {
                    needSetMapTouchDevice = true;
                    break;
                } else {
                    if(oldOutput->currentMode()->size() != output->currentMode()->size()) {
                        needSetMapTouchDevice = true;
                    }
                }
            }
        }
    } else {
        needSetMapTouchDevice = true;
    }
    oldOutputs = m_outputsConfig->data()->clone();
    if (needSetMapTouchDevice) {
        onlyCalibrate();
    }
}

void XrandrManager::onlyCalibrate()
{
    TouchCalibrate calibrate;
    calibrate.calibrate();
}

double XrandrManager::getSingleScreenScale(QString &errorMsg)
{
    int enableCount = 0;
    double scale;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }

        if (output->isEnabled()) {
            enableCount++;
            scale = output->scale();
        }
    }

    if (enableCount != 1) {
        errorMsg = QString("screen count beyond 1");
        return 1;
    }
    return scale;
}

bool XrandrManager::setSingleScreenScale(double scale ,QString &errorMsg)
{
    int enableCount = 0;
    QSize screenSize;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }

        if (output->isEnabled()) {
            enableCount++;
            screenSize = output->currentMode()->size();
        }
    }

    if (enableCount != 1) {
        errorMsg = QString("screen count beyond 1");
        return false;
    }

    if(screenSize.width() > 3840) {//华为产线无法获取尺寸，客户需求时150%。使用150%缩放。
        if (scale > 2) {
            errorMsg = QString("%1 no support %1").arg(screenSize.width()).arg(scale);
            return false;
        }
    } else if (screenSize.width() > 2160) {
        if (scale > 1.75) {
            errorMsg = QString("%1 no support %1").arg(screenSize.width()).arg(scale);
            return false;
        }
    } else {
        if (scale > 1.5) {
            errorMsg = QString("%1 no support %1").arg(screenSize.width()).arg(scale);
            return false;
        }
    }

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }

        if (output->isEnabled()) {
            output->setScale(scale);
        }
    }

    applyConfig();
    return true;
}

void XrandrManager::sendOutputsModeToDbus()
{
    m_xrandrDbus->sendModeChangeSignal(discernScreenMode());
}

void XrandrManager::applyConfig()
{
    // 记录所有输出配置信息用于调试
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        SYS_LOG_SHOW_OUTPUT(output);
    }

    if (m_outputsConfig->canBeApplied()) {
        m_isSetting = true;
        disconnectAllOutputSlot();

        KScreen::SetConfigOperation* operation = new KScreen::SetConfigOperation(m_outputsConfig->data());
        connect(operation, &KScreen::SetConfigOperation::finished,
                this, [this, operation] {
            // 操作完成后清理资源
            operation->deleteLater();

            SYS_LOG(LOG_INFO, "Configuration application completed");

            // 执行配置后的相关操作
            calibrateTouchDevice();
            sendOutputsModeToDbus();
            m_outputsConfig->setScreenMode(m_outputModeEnum.valueToKey(discernScreenMode()));
            writeConfig();

            m_isSetting = false;
            connectAllOutputSlot();

            // 验证配置是否生效，如有差异则启动重试机制
            verifyAndRetryConfig();
        });
    } else {
        USD_LOG(LOG_ERR, "Configuration cannot be applied - invalid state");
        m_isSetting = false;

        // 记录当前输出状态用于调试
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            SYS_LOG_SHOW_OUTPUT(output);
        }

        // 执行输出添加处理
        doOutputAdded(nullptr);
    }
}

void XrandrManager::writeConfig()
{
    bool isNeedSave = true;
    uint connectOutput = 0;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }
        USD_LOG_SHOW_OUTPUT(output);
    }
    do {
        if (!m_isInitFinish) {
            USD_LOG(LOG_DEBUG,"skip save during init...");
            return;
        }

        if (!UsdBaseClass::isJJW7200() ) {
            USD_LOG(LOG_DEBUG,"skip jjw fake output1");
            break;
        }

        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (!output->isConnected()) {
                continue;
            }
            connectOutput++;
            USD_LOG_SHOW_OUTPUT(output);
        }
        if (connectOutput != 1) {
            USD_LOG(LOG_DEBUG,"skip jjw fake output2:%d",connectOutput);
            break;
        }

        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (!output->isConnected()) {
                continue;
            }

//            if (output->name().contains("VGA",Qt::CaseInsensitive))
            {
                if (output->modes().count() < 3) {
                    isNeedSave = false;
                    USD_LOG(LOG_DEBUG,"skip jjw fake output!");
                }
                break;
            }
        }
    }while(0);
    setMaxScaleToXsettings();
    m_outputsConfig->writeFile(false);
}

//用于外置显卡，当外置显卡插拔时会有此事件产生
void XrandrManager::doOutputAdded(const KScreen::OutputPtr &output)
{
    m_isSetting = true;
    m_isInitFinish = false;
    disconnectAllOutputSlot();
    m_screenSignalTimer->stop();
    m_outputsInitTimer->start(10);
    if (output != nullptr) {
        USD_LOG_SHOW_OUTPUT(output);
        m_xrandrDbus->sendScreenAddedSignal(output->name());
    }
}


void XrandrManager::doOutputRemoved(int outputId)
{
    if (!m_outputsConfig->data()->outputs().contains(outputId)) {
        return;
    }
    disconnectAllOutputSlot();
    m_outputsInitTimer->start(10);
    m_isInitFinish = false;
}

/*
 *
 *接入时没有配置文件的处理流程：
 *单屏：最优分辨率。
 *多屏幕：镜像模式。
 *
*/
void XrandrManager::outputConnectedWithoutConfigFile(KScreen::Output *newOutput, char outputCount)
{
    Q_UNUSED(newOutput);
    USD_LOG_SHOW_PARAM1(outputCount);
    if (1 >= outputCount) {//单屏接入时需要设置模式，主屏
        setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::firstScreenMode));
    } else {
        QString defaultMode = m_xrandrSettings->get(DEFAULT_MODE_KEY).toString();
        if (defaultMode == m_outputModeEnum.key(UsdBaseClass::eScreenMode::showKDS)) {
            USD_LOG(LOG_DEBUG,"showKDS");
            showKDS();
            return;
        }
        setOutputsMode(defaultMode);
    }

}

void XrandrManager::lightLastScreen()
{
    int enableCount = 0;
    int connectCount = 0;

    Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
        if (output->isConnected()){
            connectCount++;
        }
        if (output->isEnabled()){
            enableCount++;
        }
    }
    if (connectCount==1 && enableCount==0){
        Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
            if (output->isConnected()){
                output->setEnabled(true);
                break;
            }
        }
    }
}

int XrandrManager::getCurrentRotation()
{
    uint8_t ret = 1;
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_STATUSMANAGER_NAME,
                                                          DBUS_STATUSMANAGER_PATH,
                                                          DBUS_STATUSMANAGER_NAME,
                                                          DBUS_STATUSMANAGER_GET_ROTATION);

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if (response.arguments().isEmpty() == false) {
            QString value = response.arguments().takeFirst().toString();
            USD_LOG(LOG_DEBUG, "get mode :%s", value.toLatin1().data());
            if (value == "normal") {
                ret = 1;
            } else if (value == "left") {
                 ret = 2;
            } else if (value == "upside-down") {
                  ret = 4;
            } else if  (value == "right") {
                   ret = 8;
            } else {
                USD_LOG(LOG_DEBUG,"Find a error !!! value%s",value.toLatin1().data());
                return ret = 1;
            }
        }
    }
    return ret;
}

/*
 *
 * -1:无接口
 * 0:PC模式
 * 1：tablet模式
 *
*/
int XrandrManager::getCurrentMode()
{
    QDBusMessage message = QDBusMessage::createMethodCall(DBUS_STATUSMANAGER_NAME,
                                                          DBUS_STATUSMANAGER_PATH,
                                                          DBUS_STATUSMANAGER_NAME,
                                                          DBUS_STATUSMANAGER_GET_MODE);

    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if(response.arguments().isEmpty() == false) {
            bool value = response.arguments().takeFirst().toBool();
            return value;
        }
    }
    return -1;
}

void XrandrManager::doOutputChanged(KScreen::Output *senderOutput)
{
    char outputConnectCount = 0;
    bool hadFindOutput = false;
    USD_LOG_SHOW_OUTPUT(senderOutput);
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
//        if (/*output->name()==senderOutput->name() && output->hash() != senderOutput->hash()*/)
        if (senderOutput->id() == output->id()){
            senderOutput->setEnabled(senderOutput->isConnected());
            m_outputsConfig->data()->removeOutput(output->id());
            m_outputsConfig->data()->addOutput(senderOutput->clone());
            hadFindOutput = true;
            USD_LOG_SHOW_OUTPUT(output);
            break;
        }
    }

    if (false == hadFindOutput) {
        USD_LOG(LOG_DEBUG,"we find bug...");
    }

    Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
        if (senderOutput->id() == output->id()) {//这里只设置connect，enbale由配置设置。
            output->setEnabled(senderOutput->isConnected());
            output->setConnected(senderOutput->isConnected());
            output->setModes(senderOutput->modes());
            output->setPreferredModes(senderOutput->preferredModes());
            if (UsdBaseClass::isWayland()) {
                output->setScale(senderOutput->scale());
            }
            USD_LOG_SHOW_OUTPUT(output);
       }
        if (output->isConnected()) {
            outputConnectCount++;
        }
    }

    if (UsdBaseClass::isTablet() && getCurrentMode()) {//平板项目并且是平板模式
        int ret = getCurrentMode();
        USD_LOG(LOG_DEBUG,"table mode need't use config file");
        if (0 < ret) {
            //tablet模式
              setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::cloneScreenMode));
        } else {
            //PC模式
              setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::extendScreenMode));
        }
    } else {//非tablet项目无此接口
        if (false == m_outputsConfig->fileExists()) {
            outputConnectedWithoutConfigFile(senderOutput, outputConnectCount);
        } else {
            if (outputConnectCount) {
                std::unique_ptr<xrandrConfig> MonitoredConfig  = m_outputsConfig->readFile(false);
                if (MonitoredConfig!=nullptr) {
                    m_outputsConfig = std::move(MonitoredConfig);
                } else {
                    if (outputConnectCount>1) {
                        if (!checkSettable(UsdBaseClass::eScreenMode::cloneScreenMode)) {
                            if (!checkSettable(UsdBaseClass::eScreenMode::extendScreenMode)) {
                                return setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::cloneScreenMode));
                            }
                        }
                    }
                    USD_LOG(LOG_DEBUG,"read config file error! ");
                }
            }
        }
    }

    USD_LOG(LOG_DEBUG,"read config file success!");

    applyConfig();
    if (UsdBaseClass::isJJW7200()) {
        QTimer::singleShot(3*1000, this, [this](){
            applyConfig();
            USD_LOG(LOG_DEBUG,"signalShot......");
        });
    }
}

void XrandrManager::doOutputModesChanged()
{
//TODO: just handle modesChanges signal for cloud desktop
/*
 * make sure the size in Kscreen config is smaller than max screen size
 * if ouputname != nullptr set this ouput mode is preffer mode,
*/
/*
 * 确保kscreen中的size小于screen的最大尺寸，
 * 如果ouputname不为空，调整output的最佳分辨率，重新进行设置，并计算匹配size，
 * 如果不符合标准则调整另一个屏幕的大小。
 * 如果output为空，则屏幕均使用最佳分辨率，如果无最佳分辨率就用最大最适合。如果第三个屏幕无法接入，则不进行处理。。
 * 目前只按照两个屏幕进行处理。
 * 有时模式改变会一次性给出两个屏幕的信号，就需要在这里同时处理多块屏幕!。
 * 第一步：找到坐标为0，0的屏幕。
 * 第二步：横排所有屏幕
*/
    int newPosX = 0;
    int findOutputCounect = 0;
    //找出pos(0.0)的屏幕大小
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }
        //不能在这里获取size大小，特殊情况下currentMode->size会报错
        if (output->pos() == QPoint(0,0)) {
            findOutputCounect++;
            if (m_modesChangeOutputs.contains(output->name()) &&
                    output->modes().contains(output->preferredModeId())) {
                output->setCurrentModeId(output->preferredModeId());
            }
            newPosX += output->currentMode()->size().width();
            break;
        }
    }

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected() || !output->isEnabled()) {
            continue;
        }
        if (output->pos() != QPoint(0,0)) {
            output->setPos(QPoint(newPosX,0));
            if (m_modesChangeOutputs.contains(output->name()) &&
                    output->modes().contains(output->preferredModeId())) {
                 output->setCurrentModeId(output->preferredModeId());
            }
            newPosX += output->currentMode()->size().width();
        }
    }
    applyConfig();
}

//处理来自控制面板的操作,保存配置
void XrandrManager::doSaveConfigTimeOut()
{
    int enableScreenCount = 0;
    m_screenSignalTimer->stop();

    if (m_outputsChangedSignal & eScreenSignal::isModesChanged && !(m_outputsChangedSignal & eScreenSignal::isConnectedChanged)) {

        if (!(m_outputsChangedSignal & eScreenSignal::isCurrentModeIdChanged)) {
            USD_LOG(LOG_DEBUG,"handle modes changed signal");
            doOutputModesChanged();
        } else {
            USD_LOG(LOG_DEBUG,"skip modes changed signal cuz get 'currentMode'");
        }
        m_modesChangeOutputs.clear();
        m_outputsChangedSignal = eScreenSignal::isNone;
        return;
    }

    if (m_outputsChangedSignal&(eScreenSignal::isConnectedChanged|eScreenSignal::isOutputChanged)) {
        USD_LOG(LOG_DEBUG,"skip save config");
        m_applyConfigWhenSave = false;
        m_outputsChangedSignal = eScreenSignal::isNone;
        return;
    }

    m_outputsChangedSignal = eScreenSignal::isNone;
    if (false == m_applyConfigWhenSave) {
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (output->isEnabled()) {
                enableScreenCount++;
            }
        }

        if (0 == enableScreenCount) {//When user disable last one connected screen USD must enable the screen.
            m_applyConfigWhenSave = true;
            m_screenSignalTimer->start(SAVE_CONFIG_TIME*5);
            return;
        }
    }

    if (m_applyConfigWhenSave) {
        m_applyConfigWhenSave = false;
        setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::firstScreenMode));
    } else {
        Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
            USD_LOG_SHOW_OUTPUT(output);
        }
        m_outputsConfig->setScreenMode(m_outputModeEnum.valueToKey(discernScreenMode()));
        writeConfig();

//        SetTouchscreenCursorRotation();//When other app chenge screen'param usd must remap touch device
        calibrateTouchDevice();
        sendOutputsModeToDbus();
    }
}

QString XrandrManager::getOutputsInfo()
{
    return m_outputsConfig->getScreensParam();
}

int XrandrManager::initAllOutputs()
{
    char connectedOutputCount = 0;
    if (m_configPtr) {
        KScreen::ConfigMonitor::instance()->removeConfig(m_configPtr);
        for (const KScreen::OutputPtr &output : m_configPtr->outputs()) {
            output->disconnect(this);
        }
        m_configPtr->disconnect(this);
    }
    m_configPtr = std::move(m_outputsConfig->data());
    USD_LOG(LOG_DEBUG,"init...");
    connectedOutputCount = connectAllOutputSlot();


    KScreen::ConfigMonitor::instance()->addConfig(m_configPtr);

    connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged,
            this, &XrandrManager::doOutputsConfigurationChanged, Qt::UniqueConnection);

    connect(m_configPtr.data(), SIGNAL(outputAdded(KScreen::OutputPtr)),
            this, SLOT(doOutputAdded(KScreen::OutputPtr)));

    connect(m_configPtr.data(), SIGNAL(outputRemoved(int)),
            this, SLOT(doOutputRemoved(int)));

    USD_LOG(LOG_DEBUG,"read config:%s.",m_outputsConfig->filePath().toLatin1().data());

    if (m_outputsConfig->fileExists()) {
        int needSetParamWhenStartup = realTimeConfigHadDiffWithConfig();
        USD_LOG(LOG_DEBUG,"needSetParamWhenStartup:%d",needSetParamWhenStartup);
        if (needSetParamWhenStartup == -1) {
            USD_LOG(LOG_DEBUG,"config a error");
            setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::cloneScreenMode));
            return connectedOutputCount;
        }
        else if (needSetParamWhenStartup == 1) {
            applyConfig();
        } else {
            calibrateTouchDevice();
            m_isSetting = false;
        }
    } else {
        setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::cloneScreenMode));
    }

    return connectedOutputCount;
}

bool XrandrManager::checkPrimaryOutputsIsSetable()
{
    int connecedScreenCount = 0;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()){
        if (output->isConnected()){
            connecedScreenCount++;
        }
    }

    if (connecedScreenCount < 2) {
        USD_LOG(LOG_DEBUG, "skip set command cuz ouputs count :%d connected:%d",m_outputsConfig->data()->outputs().count(), connecedScreenCount);
        return false;
    }

    if (nullptr == m_outputsConfig->data()->primaryOutput()){
        USD_LOG(LOG_DEBUG,"can't find primary screen.");
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (output->isConnected()) {
                output->setPrimary(true);
                output->setEnabled(true);
                USD_LOG(LOG_DEBUG,"set %s as primary screen.",output->name().toLatin1().data());
                break;
            }
        }
    }
    return true;
}

bool XrandrManager::readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode eMode)
{
    if (UsdBaseClass::isTableWithDBus()) {
        return false;
    }

    m_outputsConfig->setScreenMode(m_outputModeEnum.valueToKey(eMode));
    if (m_outputsConfig->fileScreenModeExists(m_outputModeEnum.valueToKey(eMode))) {
        std::unique_ptr<xrandrConfig> MonitoredConfig = m_outputsConfig->readFile(true);
        if (MonitoredConfig == nullptr) {
            USD_LOG(LOG_DEBUG,"config a error");
            return false;
        } else {
            m_outputsConfig = std::move(MonitoredConfig);
            if (checkSettable(eMode)) {
                applyConfig();
                return true;
            }
        }
    }
    return false;
}

bool XrandrManager::checkSettable(UsdBaseClass::eScreenMode eMode)
{
    QList<QRect> listQRect;
    int x=0;
    int y=0;
    bool isSameRect = true;

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()){
        if (!output->isConnected() || !output->isEnabled()){
            continue;
        }
        listQRect<<output->geometry();
        x += output->geometry().x();
        y += output->geometry().y();
    }

    for (int i = 0; i < listQRect.size()-1; i++){
        if (listQRect.at(i) != listQRect.at(i+1)) {
            isSameRect = false;
        }
    }

    if (eMode == UsdBaseClass::eScreenMode::cloneScreenMode) {
        if (!isSameRect) {
            return false;
        }
    } else if (eMode == UsdBaseClass::eScreenMode::extendScreenMode) {
        if (isSameRect || (x==y && x==0)) {
            return false;
        }
    }
    return true;
}

void XrandrManager::setOutputsModeToCloneWithPreferredMode()
{
    bool hadFindFirstScreen = false;

    QList<QSize> slist;
    if (false == checkPrimaryOutputsIsSetable()) {
        return;
    }

    if (readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode::cloneScreenMode)) {
        return;
    }
    slist.clear();
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (false == output->isConnected()) {
            continue;
        }
        output->setScale(1);
        output->setEnabled(true);
        output->setPos(QPoint(0,0));
        output->setRotation(static_cast<KScreen::Output::Rotation>(1));
        if (false == hadFindFirstScreen) {
            hadFindFirstScreen = true;
            Q_FOREACH(auto primaryMode, output->modes()) {
                if (!slist.contains(primaryMode->size())) {
                    slist.append(primaryMode->size());
                }
            }
            continue;
        }

        QList<QSize> templist = slist;
        for (auto size: slist) {
            bool findSameSize = false;
            USD_LOG(LOG_DEBUG,"check %dx%d over",size.width(), size.height());
            Q_FOREACH(auto minjorScreenMode, output->modes()) {
                if (minjorScreenMode->size() == size) {
                    findSameSize = true;
                }
            }   

            if (false == findSameSize) {
                templist.removeAll(size);
                USD_LOG(LOG_DEBUG,"remove %dx%d over",size.width(), size.height());
                qDebug()<<templist;
            }

            USD_LOG(LOG_DEBUG,"check over");
        }
        slist = templist;

    }

    if(slist.count() > 0) {//镜像
        qSort(slist.begin(), slist.end(),[](const QSize &a, const QSize &b){
            if (a.width() == b.width())
                return a.height() > b.height();
            return a.width() > b.width();
        });//降序
    }
    double minScale = 999;
    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (false == output->isConnected()) {
            continue;
        }
        if (output->preferredMode()->size() == slist[0]) {
            output->setCurrentModeId(output->preferredMode()->id());
            if (UsdBaseClass::isWayland()) {
                output->setScale(getPreferredScale(output));
                minScale = minScale > output->scale() ? output->scale() : minScale;
            }
            continue;
        }

        Q_FOREACH(auto minjorScreenMode, output->modes()) {
            if (minjorScreenMode->size() == slist[0]) {
                output->setCurrentModeId(minjorScreenMode->id());
                if (UsdBaseClass::isWayland()) {
                    output->setScale(UsdBaseClass::getPreferredScale(output->sizeMm().width(), output->sizeMm().height(),
                                                                      minjorScreenMode->size().width(), minjorScreenMode->size().height()));
                    minScale = minScale > output->scale() ? output->scale() : minScale;
                }
                break;
            }
        }
    }

    if (UsdBaseClass::isWayland()) {
        Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
            if (false == output->isConnected()) {
                continue;
            }
            output->setScale(minScale);
        }
    }

    if (0 == slist.count()) {
        setOutputsMode(m_outputModeEnum.key(UsdBaseClass::eScreenMode::extendScreenMode));
    } else {
        applyConfig();
    }
}

void XrandrManager::setOutputsModeToFirst(bool isFirstMode)
{
    int posX = 0;
    bool hadFindFirstScreen = false;
    bool hadSetPrimary = false;

    bool hadOpenOneOutput = false;

    if (isFirstMode) {
        if (readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode::firstScreenMode)) {
            /*
             * 第一屏模式下一定要保证首个屏幕是开启状态。
             * 升级驱动或者wayland环境插拔顺序不一样会导致顺序变化。
            */
            if (m_outputsConfig->data()->outputs().first()->isEnabled()) {
                return;
            }
            SYS_LOG(LOG_DEBUG, "%s enable state:%d",qPrintable(m_outputsConfig->data()->outputs().first()->name()),
                    m_outputsConfig->data()->outputs().first()->isEnabled());

            SYS_LOG(LOG_DEBUG,"%s error, reset first firstScreenMode.",
                    qPrintable(m_outputsConfig->globalModeFilePath(m_outputModeEnum.valueToKey(UsdBaseClass::eScreenMode::firstScreenMode))));
        }
    } else {
        if (readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode::secondScreenMode)) {
            /*
             * 第二屏模式下一定要保证首个屏幕是关闭状态。
             * 升级驱动或者wayland环境插拔顺序不一样会导致顺序变化。
            */
            if (!m_outputsConfig->data()->outputs().first()->isEnabled()) {
                return;
            }
            SYS_LOG(LOG_DEBUG, "%s enable state:%d",qPrintable(m_outputsConfig->data()->outputs().first()->name()),
                    m_outputsConfig->data()->outputs().first()->isEnabled());
            SYS_LOG(LOG_DEBUG,"%s error, reset first secondScreenMode.",
                    qPrintable(m_outputsConfig->globalModeFilePath(m_outputModeEnum.valueToKey(UsdBaseClass::eScreenMode::secondScreenMode))));
        }
    }

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            output->setEnabled(false);
            continue;
        }

        // 设置输出启用状态
        bool shouldEnable = false;
        if (isFirstMode) {
            shouldEnable = !hadFindFirstScreen;
            hadFindFirstScreen = true;
        } else {
            if (hadFindFirstScreen) {
                shouldEnable = !hadOpenOneOutput;
                hadOpenOneOutput = true;
            } else {
                hadFindFirstScreen = true;
                shouldEnable = false;
            }
        }
        output->setEnabled(shouldEnable);

        if (shouldEnable) {
            // 设置主显示器
            bool isPrimary = !hadSetPrimary;
            output->setPrimary(isPrimary);
            hadSetPrimary = true;

            // 设置显示模式
            output->setCurrentModeId(output->preferredModeId());

            // Wayland特定设置
            if (UsdBaseClass::isWayland()) {
                output->setScale(getPreferredScale(output));
            }

            // 设置位置
            output->setPos(QPoint(posX, 0));
            posX += output->size().width();
        }

        USD_LOG_SHOW_OUTPUT(output);
    }
    applyConfig();
}

void XrandrManager::setOutputsModeToExtend()
{
    int primaryX = 0;
    int screenSize = 0;
    int singleMaxWidth = 0;
    float refreshRate = 0.0;
    bool hadFindPrimay = false;
    int connectedScreens;

    if (false == checkPrimaryOutputsIsSetable()) {
        return;
    }

    if (readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode::extendScreenMode)) {
        return;
    }

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()){
            continue;
        }
        connectedScreens++;
    }


    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {

            if (!output->isConnected()){
                continue;
            }
            if (hadFindPrimay) {
                output->setPrimary(false);
                continue;
            }
            if (!output->name().contains("eDP-1")) {//考虑   pnZXECRB项目中内屏为 DisplayPort-0
                output->setPrimary(false);
                continue;
            }

            hadFindPrimay = true;
            output->setPrimary(true);
            output->setEnabled(true);
            output->setRotation(static_cast<KScreen::Output::Rotation>(1));
            screenSize = 0;
            refreshRate = 0.0;

            Q_FOREACH(auto Mode, output->modes()) {
                if (Mode->size().width()*Mode->size().height() < screenSize){
                    continue;
                } else if (Mode->size().width()*Mode->size().height() == screenSize) {
                    if (Mode->refreshRate() <= refreshRate) {
                        continue;
                    }
                }

                refreshRate = Mode->refreshRate();
                screenSize = Mode->size().width()*Mode->size().height();
                output->setCurrentModeId(Mode->id());
                if (Mode->size().width() > singleMaxWidth) {
                    singleMaxWidth = Mode->size().width();
                }
            }

            output->setPos(QPoint(0,0));
            primaryX += singleMaxWidth;
            USD_LOG_SHOW_OUTPUT(output);
        }

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        screenSize = 0;
        refreshRate = 0.0;
        singleMaxWidth = 0;

        if (output->isConnected()){
            output->setEnabled(true);
        } else {
            output->setEnabled(false);
            continue;
        }

        if (!hadFindPrimay) {
            output->setPrimary(true);
            hadFindPrimay = true;
        } else {
            if (output->isPrimary()){
                continue;
            }
        }

        output->setEnabled(true);
        output->setRotation(static_cast<KScreen::Output::Rotation>(1));

        Q_FOREACH(auto Mode, output->modes()){
            if (Mode->size().width()*Mode->size().height() < screenSize){
                continue;
            } else if (Mode->size().width()*Mode->size().height() == screenSize) {
                if (Mode->refreshRate() <= refreshRate) {
                    continue;
                }
            }

            refreshRate = Mode->refreshRate();
            screenSize = Mode->size().width()*Mode->size().height();
            output->setCurrentModeId(Mode->id());
            if (Mode->size().width() > singleMaxWidth) {
                singleMaxWidth = Mode->size().width();
            }
        }
        if (UsdBaseClass::isTablet()) {
            output->setRotation(static_cast<KScreen::Output::Rotation>(getCurrentRotation()));
        }
        output->setPos(QPoint(primaryX,0));
        primaryX += singleMaxWidth;
        USD_LOG_SHOW_OUTPUT(output);
    }
    applyConfig();
}


void XrandrManager::setOutputsModeToExtendWithPreferredMode()
{
    int primaryOffsetX = 0;

    if (!checkPrimaryOutputsIsSetable()) {
        USD_LOG(LOG_WARNING, "Primary output configuration disabled");
        return;
    }

    if (readAndApplyOutputsModeFromConfig(UsdBaseClass::eScreenMode::extendScreenMode)) {
        return;
    }

    primaryOffsetX = configurePrimaryOutput();
    USD_LOG_SHOW_PARAM1(primaryOffsetX);
    configureSecondaryOutput(primaryOffsetX);
    applyConfig();
}

void XrandrManager::setOutputsParam(QString screensParam)
{
    USD_LOG(LOG_DEBUG,"param:%s", screensParam.toLatin1().data());
    std::unique_ptr<xrandrConfig> temp  = m_outputsConfig->readScreensConfigFromDbus(screensParam);
    if (nullptr != temp) {
        m_outputsConfig = std::move(temp);
    }
    applyConfig();
}


/*
 * 设置显示模式
*/
void XrandrManager::setOutputsMode(QString modeName)
{
    //检查当前屏幕数量，只有一个屏幕时不设置
    int screenConnectedCount = 0;
    int modeValue = m_outputModeEnum.keyToValue(modeName.toLatin1().data());

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (true == output->isConnected()) {
            screenConnectedCount++;
        }
    }

    if(screenConnectedCount == 0) {
        return;
    }
    SYS_LOG_SHOW_PARAM1(modeValue);
    if(screenConnectedCount <= 1) {
        modeValue = UsdBaseClass::eScreenMode::firstScreenMode;
    }
    SYS_LOG_SHOW_PARAM1(modeValue);
    switch (modeValue) {
    case UsdBaseClass::eScreenMode::cloneScreenMode:
        SYS_LOG(LOG_DEBUG,"set mode to %s",modeName.toLatin1().data());
        setOutputsModeToCloneWithPreferredMode();
        break;
    case UsdBaseClass::eScreenMode::firstScreenMode:
        SYS_LOG(LOG_DEBUG,"set mode to %s",modeName.toLatin1().data());
        setOutputsModeToFirst(true);
        break;
    case UsdBaseClass::eScreenMode::secondScreenMode:
        SYS_LOG(LOG_DEBUG,"set mode to %s",modeName.toLatin1().data());
        setOutputsModeToFirst(false);
        break;
    case UsdBaseClass::eScreenMode::extendScreenMode:
        SYS_LOG(LOG_DEBUG,"set mode to %s",modeName.toLatin1().data());
        setOutputsModeToExtendWithPreferredMode();
        break;
    default:
        USD_LOG(LOG_DEBUG,"set mode fail can't set to %s",modeName.toLatin1().data());
        return;
    }
    sendOutputsModeToDbus();
}

/*
 * 识别当前显示的模式
*/
UsdBaseClass::eScreenMode XrandrManager::discernScreenMode()
{
    // 使用结构化变量初始化
    struct ScreenInfo {
        bool enabled = false;
        QPoint pos;
        QSize size;
        bool valid = false;  // 新增有效性标志
    };

    ScreenInfo primaryScreen;
    ScreenInfo secondaryScreen;
    int connected = 0;
    int enabled = 0;

    // 使用范围for循环简化遍历
    for (const auto& output : m_outputsConfig->data()->outputs()) {
        if (output->isEnabled()) {
            ++enabled;
        }

        if (!output->isConnected()) {
            continue;
        }

        ++connected;
        if (connected == 1) {
            primaryScreen.enabled = output->isEnabled();
            primaryScreen.valid = primaryScreen.enabled && output->currentMode();
            if (primaryScreen.valid) {
                primaryScreen.pos = output->pos();
                primaryScreen.size = output->currentMode()->size();
            }
        } else if (connected == 2) {
            secondaryScreen.enabled = output->isEnabled();
            secondaryScreen.valid = secondaryScreen.enabled && output->currentMode();
            if (secondaryScreen.valid) {
                secondaryScreen.pos = output->pos();
                secondaryScreen.size = output->currentMode()->size();
            }
        }
    }

    // 模式判断逻辑
    if (primaryScreen.enabled && !secondaryScreen.enabled || connected == 1) {
        USD_LOG(LOG_DEBUG, "mode : firstScreenMode");
        return UsdBaseClass::eScreenMode::firstScreenMode;
    }

    if (!primaryScreen.enabled && secondaryScreen.enabled ||
            (connected > 2 && enabled <= 2)) {
        USD_LOG(LOG_DEBUG, "mode : secondScreenMode");
        return UsdBaseClass::eScreenMode::secondScreenMode;
    }

    if (primaryScreen.valid && secondaryScreen.valid &&
            primaryScreen.pos == secondaryScreen.pos &&
            primaryScreen.size == secondaryScreen.size) {
        USD_LOG(LOG_DEBUG, "mode : cloneScreenMode");
        return UsdBaseClass::eScreenMode::cloneScreenMode;
    }

    USD_LOG(LOG_DEBUG, "mode : extendScreenMode");
    return UsdBaseClass::eScreenMode::extendScreenMode;
}

void XrandrManager::doCalibrate(const QString screenMap)
{
    USD_LOG(LOG_DEBUG,"controlScreenMap ...");
    doRotationChanged(screenMap);
}

void XrandrManager::disableCrtc()
{
    int tempInt;

    Display	*m_pDpy;
    Window	m_rootWindow;
    XRRScreenResources  *m_pScreenRes;
    int m_screen;
    m_pDpy = XOpenDisplay (NULL);
    if (m_pDpy == NULL) {
        USD_LOG(LOG_DEBUG,"XOpenDisplay fail...");
        return ;
    }

    m_screen = DefaultScreen(m_pDpy);
    if (m_screen >= ScreenCount (m_pDpy)) {
        USD_LOG(LOG_DEBUG,"Invalid screen number %d (display has %d)",m_screen, ScreenCount(m_pDpy));
        return ;
    }

    m_rootWindow = RootWindow(m_pDpy, m_screen);
    m_pScreenRes = XRRGetScreenResources(m_pDpy, m_rootWindow);
    if (NULL == m_pScreenRes) {
        USD_LOG(LOG_DEBUG,"could not get screen resources",m_screen, ScreenCount(m_pDpy));
        return ;
    }
    if (m_pScreenRes->noutput == 0) {
        USD_LOG(LOG_DEBUG, "noutput is 0!!");
        return ;
    }
    USD_LOG(LOG_DEBUG,"initXparam success");
    for (tempInt = 0; tempInt < m_pScreenRes->ncrtc; tempInt++) {
        int ret = 0;
        ret = XRRSetCrtcConfig (m_pDpy, m_pScreenRes, m_pScreenRes->crtcs[tempInt], CurrentTime,
                                0, 0, None, RR_Rotate_0, NULL, 0);
        if (ret != RRSetConfigSuccess) {
            USD_LOG(LOG_ERR,"disable crtc:%d error! ");
        }
    }
    XCloseDisplay(m_pDpy);
    USD_LOG(LOG_DEBUG,"disable crtc  success");
}

int XrandrManager::getConnectScreensCount()
{
    Display	*m_pDpy = nullptr;
    Window	m_rootWindow;
    XRRScreenResources  *m_pScreenRes = nullptr;
    int m_screen;
    int count = 0;
    m_pDpy = XOpenDisplay (NULL);
    if (m_pDpy == NULL) {
        USD_LOG(LOG_DEBUG,"XOpenDisplay fail...");
        return 0;
    }

    m_screen = DefaultScreen(m_pDpy);
    if (m_screen >= ScreenCount (m_pDpy)) {
        USD_LOG(LOG_DEBUG,"Invalid screen number %d (display has %d)",m_screen, ScreenCount(m_pDpy));
        goto END;
    }

    m_rootWindow = RootWindow(m_pDpy, m_screen);
    m_pScreenRes = XRRGetScreenResources(m_pDpy, m_rootWindow);
    if (NULL == m_pScreenRes) {
        USD_LOG(LOG_DEBUG,"could not get screen resources",m_screen, ScreenCount(m_pDpy));
        goto END;
    }

    if (m_pScreenRes->noutput == 0) {
        USD_LOG(LOG_DEBUG, "noutput is 0!!");
        goto END;
    }

    for(int k = 0; k < m_pScreenRes->noutput; k++) {
        RROutput outputId = m_pScreenRes->outputs[k];
        XRROutputInfo	*outputInfo = XRRGetOutputInfo (m_pDpy, m_pScreenRes, outputId);
        if (outputInfo->connection == RR_Connected) {
            count++;
        }
        XRRFreeOutputInfo(outputInfo);
    }
END:
    if(m_pScreenRes != nullptr)
        XRRFreeScreenResources(m_pScreenRes);

    if (m_pDpy != nullptr)
        XCloseDisplay(m_pDpy);

    return count;
}

void XrandrManager::showKDS()
{

    QDBusInterface mediaKeys(QString::fromLatin1(DBUS_MK_NAME),
                             QString::fromLatin1(DBUS_MK_PATH),
                             QString::fromLatin1(DBUS_MK_INTERFACE),
                             QDBusConnection::sessionBus(),this);
    mediaKeys.asyncCall(DBUS_MK_DOACTION, 38 ,"");

}

void XrandrManager::disconnectAllOutputSlot()
{
    for (const KScreen::OutputPtr &output: m_configPtr->outputs()) {
        output.data()->disconnect();
    }
}

int XrandrManager::connectAllOutputSlot()
{
    int connectedOutputCount = 0;
    for (const KScreen::OutputPtr &output: m_configPtr->outputs()) {
        USD_LOG_SHOW_OUTPUT(output);
        if (output->isConnected()){
            connectedOutputCount++;
        }

        connect(output.data(), &KScreen::Output::isConnectedChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isConnectedChanged;
            SYS_LOG(LOG_DEBUG,"%s isConnectedChanged connect:%d",senderOutput->name().toLatin1().data(), senderOutput->isConnected());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            doOutputChanged(senderOutput);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::outputChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isOutputChanged;
            SYS_LOG(LOG_DEBUG,"%s outputchanged connect:%d",senderOutput->name().toLatin1().data(), senderOutput->isConnected());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_screenSignalTimer->stop();
            if (UsdBaseClass::isJJW7200()){
                USD_LOG(LOG_DEBUG,"catch a jjw7200..");
                doOutputChanged(senderOutput);
            }
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::isPrimaryChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr || !senderOutput->isEnabled()) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isPrimaryChanged;
            SYS_LOG(LOG_DEBUG,"PrimaryChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name() == senderOutput->name()) {
                    output->setPrimary(senderOutput->isPrimary());
                    break;
                }
            }
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        });

        connect(output.data(), &KScreen::Output::posChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG(LOG_DEBUG,"skip %s posChanged signal until applyConfig over",senderOutput->name().toLatin1().data());
                return;
            }

            if (m_outputsChangedSignal & (eScreenSignal::isConnectedChanged|eScreenSignal::isOutputChanged)) {
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isPosChanged;
            SYS_LOG(LOG_DEBUG,"posChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name() == senderOutput->name()) {
                    output->setPos(senderOutput->pos());
                    break;
                }
            }
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::sizeChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG(LOG_DEBUG,"skip %s sizeChanged signal until applyConfig over",senderOutput->name().toLatin1().data());
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isSizeChanged;
            SYS_LOG(LOG_DEBUG,"sizeChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::scaleChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr /*|| UsdBaseClass::isWaylandWithKscreen()*/) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG_SHOW_OUTPUT(senderOutput);
                USD_LOG(LOG_DEBUG,"skip %s scaleChanged signal until applyConfig over",senderOutput->name().toLatin1().data());
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isScaleChanged;
            SYS_LOG(LOG_DEBUG,"scaleChanged:%s %f",senderOutput->name().toLatin1().data(),senderOutput->scale());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);

            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name()==senderOutput->name()) {//这里只设置connect，enbale由配置设置。
                    output->setScale(senderOutput->scale());
                    USD_LOG_SHOW_OUTPUT(output);
                    break;
               }
            }
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::modesChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name()==senderOutput->name()) {//这里只设置connect，enbale由配置设置。
                    if (output->preferredModeId() == nullptr ||
                            senderOutput->preferredModeId() == nullptr) {
                        USD_LOG(LOG_DEBUG,"%s prefferedMode is none", senderOutput->name());
                        return;
                    }

                    /*
                     * v101中长城T350P物理机也出现这种情况了！如果是物理机存在问题，仅判断最佳是不够的。
                     * 虚拟环境使用最佳是为了适配虚拟客户端窗口的大小，
                     * 物理环境出现这种问题，必须要更新所有列表。
                     * v11同样增加此种配置
                     */
                    if (UsdBaseClass::isVirt()) {
                        if (output->preferredMode()->refreshRate() == senderOutput->preferredMode()->refreshRate() &&
                                output->preferredMode()->size() == senderOutput->preferredMode()->size()) {
                            USD_LOG(LOG_DEBUG,"had the same mode");//sometimes cloud envriment change mode would't modefy the id.
                            return;
                        }
                    }

                    output->setEnabled(senderOutput->isConnected());
                    output->setConnected(senderOutput->isConnected());
                    output->setModes(senderOutput->modes());
                    USD_LOG(LOG_DEBUG,"old mode id:%s", output->preferredModeId().toLatin1().data());
                    output->setPreferredModes(senderOutput->preferredModes());
                    USD_LOG(LOG_DEBUG,"new mode id:%s", output->preferredModeId().toLatin1().data());
                    break;
                }
            }
            m_modesChangeOutputs.append(senderOutput->name());
            m_outputsChangedSignal |= eScreenSignal::isModesChanged;

            SYS_LOG(LOG_DEBUG,"%s modesChanged",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::clonesChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }
            m_outputsChangedSignal |= eScreenSignal::isClonesChanged;
            SYS_LOG(LOG_DEBUG,"clonesChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::rotationChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG_SHOW_OUTPUT(senderOutput);
                USD_LOG(LOG_DEBUG,"skip %s rotationChanged signal until applyConfig over",senderOutput->name().toLatin1().data());
                return;
            }

            m_outputsChangedSignal |= eScreenSignal::isRotationChanged;
            SYS_LOG(LOG_DEBUG,"rotationChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name() == senderOutput->name()) {
                    output->setRotation(senderOutput->rotation());
                    break;
                }
            }

            SYS_LOG(LOG_DEBUG,"rotationChanged:%s",senderOutput->name().toLatin1().data());
            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::currentModeIdChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG_SHOW_OUTPUT(senderOutput);
                USD_LOG(LOG_DEBUG,"skip %s currentModeIdChanged signal until applyConfig over",senderOutput->name().toLatin1().data());
                return;
            }

            SYS_LOG(LOG_DEBUG,"currentModeIdChanged:%s",senderOutput->name().toLatin1().data());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_outputsChangedSignal |= eScreenSignal::isCurrentModeIdChanged;
            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name() == senderOutput->name()) {
                    output->setCurrentModeId(senderOutput->currentModeId());
                    output->setEnabled(senderOutput->isEnabled());
                    break;
                }
            }

            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);

        connect(output.data(), &KScreen::Output::isEnabledChanged, this, [this](){
            KScreen::Output *senderOutput = static_cast<KScreen::Output*> (sender());
            if (senderOutput == nullptr) {
                USD_LOG(LOG_DEBUG,"had a bug..");
                return;
            }

            if (m_isSetting) {
                USD_LOG(LOG_ERR,"skip enable Changed signal until applyConfig over");
                return;
            }

            SYS_LOG(LOG_DEBUG,"%s isEnabledChanged %d ",senderOutput->name().toLatin1().data(),senderOutput->isEnabled());
            SYS_LOG_SHOW_OUTPUT(senderOutput);
            m_outputsChangedSignal |= eScreenSignal::isEnabledChanged;

            Q_FOREACH(const KScreen::OutputPtr &output,m_outputsConfig->data()->outputs()) {
                if (output->name() == senderOutput->name()) {
                    if (output->isConnected() == senderOutput->isConnected()) {
                        output->setEnabled(senderOutput->isEnabled());
                        break;
                    }
                }
            }

            m_screenSignalTimer->start(SAVE_CONFIG_TIME);
        },Qt::QueuedConnection);
    }
    return connectedOutputCount;
}

void XrandrManager::configureSecondaryOutput(int offsetX)
{
    // 第二阶段：配置显示器布局设置主屏位置在左侧
    for (auto &output : m_outputsConfig->data()->outputs()) {
        if (!output->isConnected() ||
                !output->preferredMode()
               || output->isPrimary()) {
            USD_LOG(LOG_DEBUG, "Skipping disconnected output: %s",
                    output->name().toUtf8().constData());
            continue;
        }
        USD_LOG(LOG_DEBUG,"find secondary.");
        offsetX += configureOuputInExtendMode(output, offsetX);
    }
}

int XrandrManager::configurePrimaryOutput()
{
    int primaryOffsetX = 0;
    int connectedScreenCount = 0;
    const int configPrimaryNumber = m_xrandrSettings->get(C_SETTINGS_XRANDR_DEFAULT_PRIMARY_NUMBER).toInt();
    const QString configPrimaryName = m_xrandrSettings->get(C_SETTINGS_XRANDR_DEFAULT_PRIMARY_NAME).toString();

    USD_LOG(LOG_INFO, "Primary config - Number:%d Name:%s",
            configPrimaryNumber, configPrimaryName.toUtf8().constData());

    for (auto &output : m_outputsConfig->data()->outputs()) {
        connectedScreenCount++;
        if (!output->isConnected()) continue;

        if (output->name().compare(configPrimaryName, Qt::CaseInsensitive) == 0 ||
                (configPrimaryNumber >= 0 && connectedScreenCount-1 == configPrimaryNumber) ||
               (configPrimaryName.isEmpty() && configPrimaryNumber < 0 && output->isPrimary())) {
            USD_LOG(LOG_DEBUG,"find primary.");
            primaryOffsetX = configureOuputInExtendMode(output, primaryOffsetX);
            continue;
        }

        output->setPrimary(false);
    }

    return primaryOffsetX;
}

void XrandrManager::setMaxScaleToXsettings()
{
    if (!UsdBaseClass::isWayland()) {
        return;
    }
    double xsettingsScaleFactor = 1.0;
     for (auto &output : m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }
        if (!output->isEnabled()) {
            continue;
        }

        xsettingsScaleFactor = std::max(output->scale(), xsettingsScaleFactor);
     }

     USD_LOG_SHOW_PARAMF(xsettingsScaleFactor);
    m_pXsettingsSettings->set(SCALING_FACTOR_KEY, xsettingsScaleFactor);
}

int XrandrManager::configureOuputInExtendMode(KScreen::OutputPtr &output, int offsetX)
{
    output->setPrimary(offsetX == 0);
    output->setEnabled(true);
    output->setCurrentModeId(output->preferredModeId());
    output->setScale(getPreferredScale(output));
    output->setRotation(static_cast<KScreen::Output::Rotation>(1));
    output->setPos(QPoint(offsetX, 0));
    USD_LOG_SHOW_OUTPUT(output);
    return output->currentMode()->size().width() / output->scale();
}

int XrandrManager::realTimeConfigHadDiffWithConfig()
{
    // 读取配置文件
    std::unique_ptr<xrandrConfig> monitoredConfig = m_outputsConfig->readFile(false);
    if (!monitoredConfig) {
        USD_LOG(LOG_DEBUG, "No configuration file found");
        return -1;
    }

    m_outputsConfig = std::move(monitoredConfig);

    // 遍历所有已连接的输出设备
    for (const KScreen::OutputPtr& oldOutput : m_configPtr->outputs()) {
        if (!oldOutput->isConnected()) {
            continue;
        }

        // 查找对应的配置输出
        const KScreen::OutputPtr& configOutput = findOutputByName(oldOutput->name());
        if (!configOutput) {
            continue;
        }

        // 检查配置差异
        if (hasConfigurationChanged(oldOutput, configOutput)) {
            return 1; // 发现差异立即返回
        }
    }

    return 0; // 无差异
}

bool XrandrManager::hasConfigurationChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    // 验证输出设备的基本状态
    if (!validateOutputForComparison(current, target)) {
        return false;
    }

    // 检查所有配置属性
    const std::vector<std::function<bool()>> checks = {
        [&]() { return hasSizeChanged(current, target); },
        [&]() { return hasRotationChanged(current, target); },
        [&]() { return hasScaleChanged(current, target); },
        [&]() { return hasPrimaryStatusChanged(current, target); },
        [&]() { return hasEnabledStatusChanged(current, target); },
        [&]() { return hasPositionChanged(current, target); },
        [&]() { return hasCurrentModeChanged(current, target); }
    };

    for (const auto& check : checks) {
        if (check()) {
            USD_LOG(LOG_DEBUG, "Configuration changed for output: %s",
                   current->name().toLatin1().data());
            return true;
        }
    }

    return false;
}

bool XrandrManager::validateOutputForComparison(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    // 检查当前输出是否有可用模式
    if (current->modes().isEmpty()) {
        USD_LOG(LOG_DEBUG, "Current output %s has no modes, skip comparison",
               current->name().toLatin1().data());
        return false;
    }

    // 检查目标输出的首选模式是否有效
    if (!target->preferredMode() ||
        target->preferredMode()->size().width() == 0 ||
        target->preferredMode()->size().height() == 0) {
        SYS_LOG(LOG_WARNING, "Invalid preferred mode for output: %s",
               target->name().toLatin1().data());
        return false;
    }

    return true;
}

KScreen::OutputPtr XrandrManager::findOutputByName(const QString& name)
{
    for (const KScreen::OutputPtr& output : m_outputsConfig->data()->outputs()) {
        if (output->name() == name) {
            return output;
        }
    }
    return nullptr;
}

bool XrandrManager::hasSizeChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    if (current->size() != target->size()) {
        USD_LOG(LOG_DEBUG, "Size changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasRotationChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    if (current->rotation() != target->rotation()) {
        USD_LOG(LOG_DEBUG, "Rotation changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasScaleChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    if (current->scale() != target->scale()) {
        USD_LOG(LOG_DEBUG, "Scale changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasPrimaryStatusChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    if (current->isPrimary() != target->isPrimary()) {
        USD_LOG(LOG_DEBUG, "Primary status changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasEnabledStatusChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    if (current->isEnabled() != target->isEnabled()) {
        USD_LOG(LOG_DEBUG, "Enabled status changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasPositionChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    // 仅在两个输出都启用的情况下比较位置信息
    if (current->isEnabled() && target->isEnabled() &&
        current->pos() != target->pos()) {
        USD_LOG(LOG_DEBUG, "Position changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }
    return false;
}

bool XrandrManager::hasCurrentModeChanged(const KScreen::OutputPtr& current, const KScreen::OutputPtr& target)
{
    bool currentHasMode = !current->currentMode().isNull();
    bool targetHasMode = !target->currentMode().isNull();

    // 检查模式存在性差异
    if (currentHasMode != targetHasMode) {
        USD_LOG(LOG_DEBUG, "Current mode existence changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }

    // 检查模式尺寸差异
    if (currentHasMode && targetHasMode &&
        current->currentMode()->size() != target->currentMode()->size()) {
        USD_LOG(LOG_DEBUG, "Current mode size changed for output: %s",
               current->name().toLatin1().data());
        return true;
    }

    return false;
}

bool XrandrManager::isFirstScreenMode()
{
    int enableCount = 0;

    Q_FOREACH(const KScreen::OutputPtr &output, m_outputsConfig->data()->outputs()) {
        if (!output->isConnected()) {
            continue;
        }
        if (output->isEnabled()) {
            enableCount ++;
        }
     }

    return (enableCount == 1 &&
            m_outputsConfig->data()->connectedOutputs().count() > 2);
}

void XrandrManager::verifyAndRetryConfig()
{
    // 检查实时配置与目标配置是否存在差异
    if (realTimeConfigHadDiffWithConfig() == 1) {
        USD_LOG(LOG_WARNING, "Configuration verification failed, starting retry procedure");

        if (m_configRetryTimes < 3) {
            m_configRetryTimes++;
            USD_LOG(LOG_INFO, "Retry attempt %d/3", m_configRetryTimes);
            applyConfig();
        } else {
            USD_LOG(LOG_ERR, "Configuration retry failed after %d attempts", m_configRetryTimes);
            m_configRetryTimes = 0;

        }
    } else {
        USD_LOG(LOG_DEBUG, "Configuration verification passed");
        m_configRetryTimes = 0;
    }
}

/**
 * @brief XrandrManager::StartXrandrIdleCb
 * 开始时间回调函数
 */
void XrandrManager::active()
{
    m_acitveTimer->stop();
    connect(m_xrandrDbus, SIGNAL(setScreenModeSignal(QString)), this, SLOT(setOutputsMode(QString)));
    connect(m_outputsInitTimer,  SIGNAL(timeout()), this, SLOT(getInitialConfig()));
    m_outputsInitTimer->start(100);


    m_screenSignalTimer = new QTimer(this);
    connect(m_screenSignalTimer, SIGNAL(timeout()), this, SLOT(doSaveConfigTimeOut()));
    USD_LOG(LOG_DEBUG,"StartXrandrIdleCb ok.");
    connect(m_xrandrDbus, SIGNAL(setScreensParamSignal(QString)), this, SLOT(setOutputsParam(QString)));

#if 0
    QDBusInterface *modeChangedSignalHandle = new QDBusInterface(DBUS_XRANDR_NAME,DBUS_XRANDR_PATH,DBUS_XRANDR_INTERFACE,QDBusConnection::sessionBus(),this);

    if (modeChangedSignalHandle->isValid()) {
        connect(modeChangedSignalHandle, SIGNAL(screenModeChanged(int)), this, SLOT(screenModeChangedSignal(int)));

    } else {
        USD_LOG(LOG_ERR, "modeChangedSignalHandle");
    }

    QDBusInterface *screensChangedSignalHandle = new QDBusInterface(DBUS_XRANDR_NAME,DBUS_XRANDR_PATH,DBUS_XRANDR_INTERFACE,QDBusConnection::sessionBus(),this);

     if (screensChangedSignalHandle->isValid()) {
         connect(screensChangedSignalHandle, SIGNAL(screensParamChanged(QString)), this, SLOT(screensParamChangedSignal(QString)));
         //USD_LOG(LOG_DEBUG, "..");
     } else {
         USD_LOG(LOG_ERR, "screensChangedSignalHandle");
     }
#endif

}
