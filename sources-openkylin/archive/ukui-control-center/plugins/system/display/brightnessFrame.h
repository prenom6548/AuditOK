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
#ifndef BRIGHTNESSFRAME_H
#define BRIGHTNESSFRAME_H

#include <QFrame>
#include <QLabel>
#include <QWidget>
#include <QLabel>
#include <QMutex>
#include <QFuture>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QGSettings>
#include <kslider.h>
#include <KF5/KScreen/kscreen/output.h>
#include <KF5/KScreen/kscreen/types.h>
#include "getBrightnessThread.h"
#include "ukccframe.h"
#include "lightlabel.h"
#include <kslider.h>
#include <klabel.h>
#include "ukcccommon.h"
#include <klabel.h>

using namespace ukcc;
using namespace kdk;

class BrightnessFrame : public UkccFrame
{
    Q_OBJECT
public:
    BrightnessFrame(const QString &name, const bool &isBattery, const QString &edidHash = "", const KScreen::OutputPtr &output = nullptr, QWidget *parent = nullptr);
    ~BrightnessFrame();
    void setTextLabelName(QString text);
    void setTextLabelValue(QString text);
    void setOutputEnable(const bool &enable);
    bool getOutputEnable();
    void runConnectThread(const bool &openFlag);
    bool getSliderEnable();
    void setSliderEnable(const bool &enable);
    void setDDCBrightness(const int &value);
    void updateEdidHash(const QString &edid);
    QString getEdidHash();
    QString getOutputName();
    void setI2cbus(QString busNum);
    void setBrightnessByGamma(bool b);
    void initBrightnessGamma();
    void setSliderRange(const bool& m_enhanceBrightOpen);

Q_SIGNALS:
    void sliderEnableChanged();
    void brightnessChanged(KScreen::OutputPtr output, int value);

public Q_SLOTS:
    void usdBrightnessSlot(QString name, uint32_t brightness);

private:
    int getMaxBrightnessAC();
    void updateBrightness();
    void sliderValueChangeSlot(QString outputName);

private:
    KLabel *labelName = nullptr;
    QLabel *labelValue = nullptr;
    KSlider *slider = nullptr;
    QString outputName;          //屏幕名
    bool    outputEnable;     //该屏幕是否打开
    bool    connectFlag;  //该屏幕是否连接
    QString edidHash;
    QMutex      mLock;
    bool exitFlag;
    bool isBattery;
    LightLabel *labelMsg = nullptr;
    QString i2cBus;
    GetBrightnessThread *mGetBrightnessThread;
    bool m_brightnessByGamma = false;
    QDBusInterface *m_usdInterface = nullptr;
    bool m_changedBySlider = false;
    KScreen::OutputPtr m_output;

    QGSettings* m_powerGSettings = nullptr;
    QGSettings* m_panelGsettings = nullptr;
};

#endif // BRIGHTNESSFRAME_H
