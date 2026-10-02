/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "gamma-manager-thread.h"

GmWorkThread::GmWorkThread(QObject *parent)
{
    m_pSetTimer = new QTimer(this);
    m_pGmHelper = new GmHelper(this);


    connect(m_pGmHelper,&GmHelper::outputInitSuccess, this, [=](){
           onUserActive(true);
            QList<OutputInfo>& outputList = m_pGmHelper->getOutputInfo();
            if (outputList.count() == 0) {
                return;
            }
           m_rtTemperature = outputList[0].rtTemp;
    });

    m_pGmHelper->initOutput();
#if 0
    m_pSetTimer->setSingleShot(false);
    m_pSetTimer->setTimerType(Qt::PreciseTimer);
    connect(m_pSetTimer, SIGNAL(timeout()), this, SLOT(doSetTimer()), Qt::QueuedConnection);
#endif

    QDBusConnection::sessionBus().connect(QString(),
                                          QString(DBUS_XRANDR_PATH),
                                          DBUS_XRANDR_INTERFACE,
                                          "screenAdded", this, SLOT(doAddedScreen(QString)));

    QDBusConnection::sessionBus().connect(QString(),
                                          QString(DBUS_XRANDR_PATH),
                                          DBUS_XRANDR_INTERFACE,
                                          "screenRemoved", this, SLOT(doRemovedScreen(QString)));

    QDBusConnection::sessionBus().connect(QString(),
                                          QString(DBUS_XRANDR_PATH),
                                          DBUS_XRANDR_INTERFACE,
                                          "screenStateChanged", this, SLOT(doScreenStateChanged(QString,int)));

    // 用户状态切换信号
    QDBusConnection::sessionBus().connect(QString(),\
                                          DBUS_GC_SIGNAL_PATH,\
                                          DBUS_GC_SIGNAL_INTERFACE,\
                                          DBUS_GC_SIGNAL_SIGNAL_Active, this, SLOT(onUserActive(bool)));

    connect(this, &QThread::finished,this ,&GmWorkThread::doThreadFinish);
}

GmWorkThread::~GmWorkThread()
{
    if (m_pSetTimer) {
        delete m_pSetTimer;
        m_pSetTimer = nullptr;
    }
}

void GmWorkThread::setAllOutputsBrightness(const double brightness)
{
    m_pGmHelper->setAllOutputsBrightness(brightness);
    USD_LOG(LOG_DEBUG,"update brightness...:%d",brightness);
}

void GmWorkThread::setBrightness(const QString outputName, const double brightness)
{
    m_pGmHelper->setBrightness(outputName, brightness);
}

bool GmWorkThread::setPrimaryBrightness(const double brightness)
{
    int ret = false;
    ret = m_pGmHelper->setPrimaryBrightness(brightness);
    if (ret) {
        this->start();
    }

    return ret;
}

int GmWorkThread::getPrimaryBrightness()
{
    return m_pGmHelper->getPrimaryBrightness();
}

void GmWorkThread::setTemperature(const int temperature)
{
    m_targetTemp = temperature;
    USD_LOG(LOG_DEBUG, "update setTemperature...:%d", m_targetTemp);
}

int GmWorkThread::getTemperature()
{
    return m_targetTemp;
}

void GmWorkThread::stopWork()
{
    isExit = true;
}

OutputGammaInfoList GmWorkThread::getAllOutputGammaInfo()
{
   return m_pGmHelper->getAllOutputsInfo();
}

void GmWorkThread::run()
{
    QList<OutputInfo>& outputList = m_pGmHelper->getOutputInfo();
    if (0 == outputList.count()) {
        USD_LOG(LOG_ERR,"get error output count = 0");
        return;
    }

    for (int var = 0; var < outputList.count(); var++) {
        if (m_rtTemperature < 0) {
            m_rtTemperature = m_targetTemp = m_lastTemperature = outputList[var].rtTemp;
            USD_LOG(LOG_DEBUG,"%s-->%d--->%d",outputList[var].name.toLatin1().data(), outputList[var].rtTemp, outputList[var].rtBrightness);
        }
    }

    //已设置背光的显示器（含无需设置），其最终值需要与显示器总数相等
    int brightSetCount = 0;
    int runTimes = 0;

    m_needSendSignalScreens.clear();

    do {
        int errorTimes = 0;
        if (isExit){
            break;
        }
        m_rtTemperature = calRealTimeColorTemperature(runTimes);
        brightSetCount = calRealTimeBrightness(m_needSendSignalScreens, outputList, runTimes);
        runTimes++;
        do{
             msleep(10);
             errorTimes++;
        } while (!m_pGmHelper->setGammaWithTemp(m_rtTemperature) && errorTimes < 30);
        m_lastTemperature = m_rtTemperature;

    } while (m_targetTemp != m_rtTemperature || brightSetCount != outputList.count());
}

void GmWorkThread::calOutputRealTimeBrightness(OutputInfo &output, QSet<QString> &screenList, const int runtime)
{
     double frac = (runtime * 50.f) / (1000.f*USD_NIGHT_LIGHT_SMOOTH_SMEAR);//调整值从大逐步变小以达到渐变效果。

    if (qAbs(output.rtBrightness - output.targetBrightness) < 3 && output.rtBrightness != output.targetBrightness) {
        output.rtBrightness = output.targetBrightness;
        output.lastBrightness = output.rtBrightness;
        screenList.insert(output.name);
    } else if (output.rtBrightness != output.targetBrightness){
        output.rtBrightness = (output.targetBrightness - output.lastBrightness) * (frac*2) + output.lastBrightness;
        output.lastBrightness = output.rtBrightness;
        USD_LOG(LOG_DEBUG,"%s ---> %0.4f ---> %0.4f", output.name.toLatin1().data(), output.rtBrightness, output.targetBrightness);
        screenList.insert(output.name);
    }
}

void GmWorkThread::sendBrightnessChangedSignal(const QList<OutputInfo> &outputList,const QSet<QString> &screenList)
{
    for (OutputInfo output: outputList) {
        if (!screenList.contains(output.name)) {
            continue;
        }

        screenBrightnessChangedSignal(output.vender, output.rtBrightness);
        screenBrightnessChangedSignal(output.name, output.rtBrightness);
        if (output.isPrimary) {
            m_pGmHelper->sendPrimaryChanged(output.targetBrightness,
                                            DBUS_GC_BRIGHTNESS_SIGNAL_PRIMARYCHANGED_END);
        }
    }
}

int GmWorkThread::calRealTimeBrightness(QSet<QString> &screenList, QList<OutputInfo> &outputList, int runTime)
{

    int brightSetCount = 0;

    for (OutputInfo &output : outputList) {

        calOutputRealTimeBrightness(output, screenList, runTime);

        if((output.rtBrightness == output.targetBrightness && output.rtTemp == m_targetTemp)
                || output.connectState != RR_Connected) {
            brightSetCount++;
        }
    }
    return brightSetCount;
}

int GmWorkThread::calRealTimeColorTemperature(const int runTime)
{
    int rtColorTemperature = COLOR_TEMPERATURE_DEFAULT;
    double frac = (runTime * 50.f) / (1000.f*USD_NIGHT_LIGHT_SMOOTH_SMEAR);//调整值从大逐步变小以达到渐变效果。

    if (qAbs(m_rtTemperature - m_targetTemp) < 10.f) {
        rtColorTemperature = m_targetTemp;
    } else {
        rtColorTemperature = (m_targetTemp - m_lastTemperature) * frac + m_lastTemperature;
    }

    if (m_rtTemperature < 0) {
        rtColorTemperature = m_targetTemp;
    }
    return rtColorTemperature;
}


void GmWorkThread::doAddedScreen(QString outputName)
{
    USD_LOG(LOG_DEBUG,"output:%s added", outputName.toLatin1().data());
}

void GmWorkThread::doRemovedScreen(QString outputName)
{
    USD_LOG(LOG_DEBUG,"output:%s removed", outputName.toLatin1().data());
}


/**
 * @brief GmWorkThread::doScreenStateChanged
 * @param outputName
 * @param state
 *
 */
void GmWorkThread::doScreenStateChanged(QString outputName, int state)
{
    m_pGmHelper->updateOutputInfo(outputName, state, m_targetTemp);
    initTempValue();
    USD_LOG(LOG_DEBUG,"output:%s state:%d,output", outputName.toLatin1().data(), state);
    if (this->isRunning()) {
        return;
    }

    if (outputName == "all") {
        QTimer::singleShot(1*1000, [=](){
           this->start(); //#205251, reset it
        });
    }

    this->start();
}

void GmWorkThread::initTempValue()
{
    if (!UsdBaseClass::isWlcom()) {
        m_rtTemperature = COLOR_TEMPERATURE_DEFAULT;
        m_lastTemperature = COLOR_TEMPERATURE_DEFAULT;
        m_targetTemp = COLOR_TEMPERATURE_DEFAULT;
    }
    USD_LOG_SHOW_PARAM1(m_rtTemperature);
    USD_LOG_SHOW_PARAM1(m_targetTemp);
}

void GmWorkThread::enablePrimarySignal(int state)
{
    m_pGmHelper->enablePrimarySignal(state);
}

void  GmWorkThread::screenBrightnessChangedSignal(QString screenName, uint brightness)
{
    QDBusMessage notifySignal =
            QDBusMessage::createSignal(DBUS_GC_BRIGHTNESS_PATH, DBUS_GC_BRIGHTNESS_INTERFACE, DBUS_GM_SCREENCHANGED);
    notifySignal.setArguments({QVariant::fromValue((QString)screenName), QVariant::fromValue((uint)brightness)});
    QDBusConnection::sessionBus().send(notifySignal);

}

void GmWorkThread::onUserActive(bool state)
{
    USD_LOG_SHOW_PARAM1(state);
    if (false == state) {
        return;
    }
    USD_LOG(LOG_DEBUG,"ceshi start set color!");
    QList<OutputInfo>& outputList = m_pGmHelper->getOutputInfo();

    for(int k = 0; k < outputList.count(); k++) {
        int brightness = m_pGmHelper->getGlobalBrightness(outputList[k].name);

        if (brightness >= 0 && brightness <= 100 &&
                !(UsdBaseClass::upmSupportAdjustBrightness() && k == 0)) {
             outputList[k].targetBrightness = brightness;
        } else {
            outputList[k].targetBrightness = 100;

//            USD_LOG(LOG_DEBUG,"read %s %s error",MODULE_NAME,  outputList[k].name);
        }
        USD_LOG(LOG_DEBUG, "%s :%d %d %d",qPrintable(outputList[k].name), outputList[k].targetTemp
                ,outputList[k].lastTemp, outputList[k].rtTemp);
    }
    this->start();
}

void GmWorkThread::doThreadFinish()
{
    bool isFinished = true;
    USD_LOG(LOG_DEBUG, "check brightenss");
    QList<OutputInfo>& outputList = m_pGmHelper->getOutputInfo();
    for(int k = 0; k < outputList.count(); k++) {
        if (outputList[k].targetBrightness != outputList[k].rtBrightness) {
            USD_LOG(LOG_DEBUG, "check brightenss error %s-->%d-->%d", qPrintable(outputList[k].name),
                    outputList[k].targetBrightness,
                    outputList[k].rtBrightness);
            this->start();
            isFinished = false;
        }
    }

    if (isFinished && !this->isRunning()) {
        USD_LOG(LOG_DEBUG, "sendBrightnessChangedSignal.");
        sendBrightnessChangedSignal(outputList, m_needSendSignalScreens);
    }
}

