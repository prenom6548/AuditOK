/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "brightnessFrame.h"
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QGSettings>
#include <QDBusConnection>
#include <QDBusMessage>
#include <unistd.h>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <KF5/KScreen/kscreen/edid.h>
#include "ukcccommon.h"
#include "usdbrightness.h"
#include "utils.h"
#include <kysdk/applications/accessinfohelper.h>
using namespace ukcc;

#define POWER_SCHMES                     "org.ukui.power-manager"
#define POWER_KEY                        "brightness-ac"
#define POWER_KEY_C                      "brightnessAc"
#define POWER_KEY_MAX_BG "max-brightness"
#define GAMMA_FOR_BRIGHTNESS_KEY "gammaforbrightness"
#define UKUI_CONTORLCENTER_PANEL_SCHEMAS "org.ukui.control-center.panel.plugins"

BrightnessFrame::BrightnessFrame(const QString &name, const bool &isBattery, const QString &edidHash,const KScreen::OutputPtr &output, QWidget *parent)
    : UkccFrame(parent, UkccFrame::BorderRadiusStyle::None, false)
{
    m_powerGSettings = new QGSettings(QByteArray(POWER_SCHMES), QByteArray(), this);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(6);
    layout->setMargin(0);

    QHBoxLayout *layout_2 = new QHBoxLayout;
    layout->setMargin(0);

    QVBoxLayout *fLayout = new QVBoxLayout(this);
    fLayout->setContentsMargins(16, 0, 16, 0);
    fLayout->setSpacing(4);
    fLayout->addLayout(layout);
    fLayout->addLayout(layout_2);

    labelMsg = new LightLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(labelMsg, "Display", "brightnessInfoLabel" + name, "brightness information label of display");
    layout_2->addWidget(labelMsg);
    labelMsg->setDisabled(true);
    labelMsg->setText(tr("Failed to get the brightness information of this monitor"));//未能获得该显示器的亮度信息

    labelName = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(labelName, "Display", "brightnessNameLabel" + name, "brightness name label of display");
    labelName->setFixedWidth(118);

    slider = new KSlider(Qt::Horizontal, this);
    KDK_EXTEND_ALL_INFO_FORMAT(slider, "Display", "brightnessSlider" + name, "brightness slider of display");
    slider->setSliderType(KSliderType::SmoothSlider);
    slider->setPageStep(1);
    slider->setRange(0, 100);
    slider->setNodeVisible(false);

    labelValue = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(labelValue, "Display", "brightnessValue" + name, "brightness value of display");
    labelValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(labelName);
    layout->addWidget(slider);
    layout->addWidget(labelValue);

    slider->setAccessibleName(tr("Brightness"));

    this->outputEnable = true;
    this->connectFlag = true;
    this->exitFlag = false;
    this->isBattery = isBattery;
    this->outputName = name;
    this->edidHash = edidHash;
    this->i2cBus = "-1";
    this->mGetBrightnessThread = nullptr;
    this->m_output = output;

    labelValue->setText("0"); //最低亮度10,获取前显示为0
    slider->setEnabled(false); //成功连接了再改为true，否则表示无法修改亮度
    if (getTabletMode()) {
        this->setFixedHeight(TABLET_HEIGHT + 48);
    } else {
        this->setFixedHeight(PC_HEIGHT + 36);
    }
    updateBrightness();
    QByteArray id(UKUI_CONTORLCENTER_PANEL_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)) {
        m_panelGsettings = new QGSettings(id, QByteArray(), this);
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center.panel.plugins not install";
    }
    connect(this, &BrightnessFrame::tabletModeChanged, this, [=](bool isTablet) {
        if (isTablet) {
            if (labelMsg) {
                this->setFixedHeight(TABLET_HEIGHT + 48);
            } else {
                this->setFixedHeight(TABLET_HEIGHT);
            }
        } else {
            if (labelMsg) {
                this->setFixedHeight(PC_HEIGHT + 36);
            } else {
                this->setFixedHeight(PC_HEIGHT);
            }
        }
    });

    connect(slider, &QSlider::sliderReleased, this, [=](){
         UkccCommon::buriedSettings(QString("display"), QString("Brightness"), QString("settings"), QString::number(slider->value()));
    });
}

BrightnessFrame::~BrightnessFrame()
{
    exitFlag = true;
    if (mGetBrightnessThread && mGetBrightnessThread->isRunning()) {
        mGetBrightnessThread->setExit(true);
        mGetBrightnessThread->terminate();
        mGetBrightnessThread->quit();
        mGetBrightnessThread->wait();
    }
}

void BrightnessFrame::setTextLabelName(QString text)
{
    this->labelName->setText(text);
    if (slider) {
        slider->setAccessibleName(text.isEmpty() ? tr("Brightness") : text);
    }
}

void BrightnessFrame::setTextLabelValue(QString text)
{
    qDebug() << Q_FUNC_INFO << text;
    this->labelValue->setText(text + "%");
}

void BrightnessFrame::runConnectThread(const bool &openFlag)
{
    outputEnable = openFlag;
    if (false == isBattery) {
        if (m_brightnessByGamma) {
            initBrightnessGamma();
    	} else if (!mGetBrightnessThread) {
            mGetBrightnessThread = new GetBrightnessThread(this->edidHash, this->i2cBus);
            connect(mGetBrightnessThread, &GetBrightnessThread::finished, this, [=](){
                mGetBrightnessThread->deleteLater();
                mGetBrightnessThread = nullptr;
            });

            connect(mGetBrightnessThread, &GetBrightnessThread::getBrightnessFinished, this, [=](int brightnessValue){
                if (brightnessValue < 0 || !slider || exitFlag) {
                    return;
                }
                slider->blockSignals(true);
                slider->setValue(brightnessValue);
                slider->blockSignals(false);
                setTextLabelValue(QString::number(brightnessValue));
                setSliderEnable(true);
                labelMsg->hide();
                if (getTabletMode()) {
                    this->setFixedHeight(TABLET_HEIGHT);
                } else {
                    this->setFixedHeight(PC_HEIGHT);
                }
                disconnect(slider,&QSlider::valueChanged,this,0);
                connect(slider, &QSlider::valueChanged, this, [=](){
                     qDebug()<<outputName<<"brightness"<<" is changed, value = "<<slider->value();
                     setTextLabelValue(QString::number(slider->value()));
                     setDDCBrightness(slider->value());
                });
            });
            mGetBrightnessThread->start();
        }
    } else {
        QByteArray powerId(POWER_SCHMES);
        if (QGSettings::isSchemaInstalled(powerId)) {
            QGSettings *mPowerGSettings = new QGSettings(powerId, QByteArray(), this);
            if (!mPowerGSettings->keys().contains(POWER_KEY_C)) {
                setTextLabelValue("-1");
            } else {
                int brightnessValue = mPowerGSettings->get(POWER_KEY).toInt();
                setTextLabelValue(QString::number(brightnessValue));
                slider->blockSignals(true);
                slider->setValue(brightnessValue);
                slider->blockSignals(false);
                setSliderEnable(true);
                labelMsg->hide();
                if (getTabletMode()) {
                    this->setFixedHeight(TABLET_HEIGHT);
                } else {
                    this->setFixedHeight(PC_HEIGHT);
                }
                disconnect(slider,&QSlider::valueChanged,this,0);
                connect(slider, &QSlider::valueChanged, this, [=](){
                    qDebug()<<outputName<<"brightness"<<" is changed, value = "<<slider->value();
                    mPowerGSettings->blockSignals(true);
                    mPowerGSettings->set(POWER_KEY, slider->value());
                    mPowerGSettings->blockSignals(false);
                    setTextLabelValue(QString::number(mPowerGSettings->get(POWER_KEY).toInt()));
                });
                disconnect(mPowerGSettings,&QGSettings::changed,this,0);
                connect(mPowerGSettings,&QGSettings::changed,this,[=](QString key){
                   if (key == POWER_KEY_C) {
                       int value = mPowerGSettings->get(POWER_KEY).toInt();
                       slider->blockSignals(true);
                       slider->setValue(value);
                       slider->blockSignals(false);
                       setTextLabelValue(QString::number(value));
                   }
                });
            }
        }
    }
}

void BrightnessFrame::setOutputEnable(const bool &enable)
{
    outputEnable = enable;
}

bool BrightnessFrame::getSliderEnable()
{
    return slider->isEnabled();
}

void BrightnessFrame::setSliderRange(const bool& m_enhanceBrightOpen) {
    if (m_enhanceBrightOpen) {
        slider->setRange(0, 150);
    } else {
        slider->setRange(0, 100);
    }
    qDebug() << Q_FUNC_INFO << "after setRange maxium" << slider->maximum() << m_enhanceBrightOpen;
}

void BrightnessFrame::setSliderEnable(const bool &enable)
{
    if (getSliderEnable() != enable) {
        this->slider->setEnabled(enable);
        Q_EMIT sliderEnableChanged();
    }
    if (false == enable) {
        labelMsg->show();
        if (getTabletMode()) {
            this->setFixedHeight(TABLET_HEIGHT + 48);
        } else {
            this->setFixedHeight(PC_HEIGHT + 36);
        }
        slider->blockSignals(true);
        slider->setValue(0);
        slider->blockSignals(false);
        setTextLabelValue("0");
    }
    return;
}

bool BrightnessFrame::getOutputEnable()
{
    return outputEnable;
}

QString BrightnessFrame::getOutputName()
{
    return outputName;
}

void BrightnessFrame::setDDCBrightness(const int &value)
{
    if (this->edidHash == "")
        return;

    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());


    if (mLock.tryLock()) {
        ukccIfc.call("setDisplayBrightness", QString::number(value), this->edidHash, this->i2cBus);
        mLock.unlock();
    }
}

void BrightnessFrame::updateEdidHash(const QString &edid)
{
    this->edidHash = edid;
}

QString BrightnessFrame::getEdidHash()
{
    return this->edidHash;
}
void BrightnessFrame::setI2cbus(QString busNum)
{
    this->i2cBus = busNum;
    return;
}

void BrightnessFrame::setBrightnessByGamma(bool b)
{
    m_brightnessByGamma = b;
}

void BrightnessFrame::sliderValueChangeSlot(QString outputName) {
    m_changedBySlider = true;
    QDBusMessage msg = m_usdInterface->call("setScreenBrightness", "ukcc", outputName, uint(slider->value()));
    qDebug() << outputName << "gamma brightness" << " is changed, value = " << slider->value() << msg.type();
    setTextLabelValue(QString::number(slider->value()));
}

void BrightnessFrame::initBrightnessGamma()
{
    if (!m_usdInterface) {
        qRegisterMetaType<UsdBrightness>("UsdBrightness");
        qDBusRegisterMetaType<UsdBrightness>();
        m_usdInterface = new QDBusInterface("org.ukui.SettingsDaemon",
                                          "/org/ukui/SettingsDaemon/GammaManager",
                                          "org.ukui.SettingsDaemon.GammaManager",
                                          QDBusConnection::sessionBus(),
                                          this);
    }
    if (m_usdInterface->isValid()) {
        QDBusMessage replyMsg = m_usdInterface->call("getScreensGammaList", qAppName());
        if (replyMsg.type() == QDBusMessage::ErrorMessage) {
            qCritical()<<"call getScreensGammaList error:"<<replyMsg.errorMessage();
        } else {
            QDBusArgument argument = replyMsg.arguments().at(0).value<QDBusArgument>();
            UsdBrightnessList list = {};
            argument.beginArray();
            while (!argument.atEnd()) {
                UsdBrightness cfg;
                argument >> cfg;
                list.push_back(cfg);
            }
            argument.endArray();
            qDebug() << Q_FUNC_INFO << "getScreensGammaList size" << list.size();
            QString outputName = m_output.data()->outputName();
            if (!list.isEmpty()) {
                for (UsdBrightness u : list) {
                    qDebug() << Q_FUNC_INFO << "gamma screenName" << u.screenName << "kscreen outputName" << outputName << "kscreen name" << m_output.data()->name();
                    if (u.screenName == outputName) {
                        int brightnessValue = u.gammaB;
                        qDebug() << Q_FUNC_INFO << "gamma brightness" << brightnessValue;
                        setTextLabelValue(QString::number(brightnessValue));
                        slider->blockSignals(true);
                        slider->setValue(brightnessValue);
                        slider->blockSignals(false);
                        setSliderEnable(true);
                        labelMsg->hide();
                        disconnect(slider,&QSlider::valueChanged,this,0);
                        connect(slider, &QSlider::valueChanged, this, [=, outputName](){
                            sliderValueChangeSlot(outputName);
                        });
                        QDBusConnection::sessionBus().disconnect("org.ukui.SettingsDaemon",
                                                                 "/GlobalBrightness",
                                                                 "org.ukui.SettingsDaemon.Brightness",
                                                                 "screenBrightnessChanged",
                                                                 this,
                                                                 SLOT(usdBrightnessSlot(QString, uint32_t)));

                        QDBusConnection::sessionBus().connect("org.ukui.SettingsDaemon",
                                                              "/GlobalBrightness",
                                                              "org.ukui.SettingsDaemon.Brightness",
                                                              "screenBrightnessChanged",
                                                              this,
                                                              SLOT(usdBrightnessSlot(QString, uint32_t)));
                        break;
                    }
                }
            }
        }
    } else {
        qCritical() << "interface org.ukui.SettingsDaemon error:" << m_usdInterface->lastError();
    }
}

void BrightnessFrame::usdBrightnessSlot(QString name, uint32_t brightness)
{

    QString curOutputName = m_output.data()->outputName();
    qDebug() << "gamma screenName" << name << "gamma brightness" << brightness << outputName << "kscreen outputName" << curOutputName;
    if ((name == outputName || name == curOutputName) && !m_changedBySlider) {
        setTextLabelValue(QString::number(brightness));
        slider->blockSignals(true);
        slider->setValue(brightness);
        slider->blockSignals(false);
    }
    m_changedBySlider = false;
}

int BrightnessFrame::getMaxBrightnessAC()
{
    int maxBrightness = -1;
    if (m_powerGSettings->keys().contains("maxBrightness") && isBattery) {
        maxBrightness = m_powerGSettings->get(POWER_KEY_MAX_BG).toInt();
    }
    return ((maxBrightness <= 100 && maxBrightness >=0) ? maxBrightness : -1);
}

void BrightnessFrame::updateBrightness()
{
    int maxBrightness = getMaxBrightnessAC();
    if ((maxBrightness != -1) && isBattery) {
        slider->setRange(0, maxBrightness);
    }
}
