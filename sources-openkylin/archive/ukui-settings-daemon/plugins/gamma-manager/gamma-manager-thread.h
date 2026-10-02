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

#ifndef GAMMAMANANGERTHREAD_H
#define GAMMAMANANGERTHREAD_H
#include <QThread>
#include <QMutex>
#include <QX11Info>
#include "gamma-manager-define.h"
#include "gamma-manager-adaptor.h"
#include "gamma-manager-helper.h"



class GmWorkThread: public QThread
{
    Q_OBJECT
public:
    GmWorkThread(QObject *parent = nullptr);
    ~GmWorkThread();
    /**
     * @brief setAllOutputsBrightness
     * @param brightness
     */
    void setAllOutputsBrightness(const double brightness);

    /**
     * @brief setBrightness
     * @param outputName
     * @param brightness
     */
    void setBrightness(const QString outputName, const double brightness);

    /**
     * @brief setPrimaryBrightness
     * @param outputName
     * @param brightness
     */
    bool setPrimaryBrightness(const double brightness);

    /**
     * @brief getBrightness
     * @return
     */
    int getPrimaryBrightness();

    /**
     * @brief setTemperature
     * @param temperature
     */
    void setTemperature(const int temperature);

    /**
     * @brief getTemperature
     * @return
     */
    int getTemperature();

    /**
     * @brief stopWork
     */
    void stopWork();

    /**
     * @brief getAllOutputGammaInfo
     * @return
     */
    OutputGammaInfoList getAllOutputGammaInfo();

    /**
     * @brief enablePrimarySignal
     * @param state
     * @return
     */
    void enablePrimarySignal(int state);

    /**
     * @brief sendScreenBrightnessChanged
     * @param screenName
     * @param brightness
     * @return
     */
    void screenBrightnessChangedSignal(QString screenName, uint brightness);
private Q_SLOTS:
    /**
     * @brief doAddedScreen
     * @param outputName
     */
    void doAddedScreen(QString outputName);

    /**
     * @brief doRemovedScreen
     * @param outputName
     */
    void doRemovedScreen(QString outputName);

    /**
     * @brief doScreenStateChanged
     * @param outputName
     * @param state
     */
    void doScreenStateChanged(QString outputName, int state);

    /**
     * @brief initTempValue 由于wlcom可以存储色温与亮度，所以需要一个接口处理。
     */
    void initTempValue();

    /**
     * @brief onUserActive 同步登入的亮度
     * @param state
     */
    void onUserActive(bool state);

    /**
     * @brief doThreadFinish 线程结束后重新检查实时亮度与目标亮度是否一致
     */
    void doThreadFinish();
protected:
    void run();
private:

    void calOutputRealTimeBrightness(OutputInfo &output, QSet<QString> &screenList, const int runtime);
    int calRealTimeBrightness(QSet<QString> &screenList, QList<OutputInfo>& outputList, const int runTime);
    int calRealTimeColorTemperature(int runTime);

    void sendBrightnessChangedSignal(const QList<OutputInfo>& outputList,const QSet<QString> &screenList);
private:
    bool isExit = false;
    int m_targetTemp = COLOR_TEMPERATURE_DEFAULT;
    int m_lastTemperature = COLOR_TEMPERATURE_DEFAULT;
    int m_rtTemperature = COLOR_TEMPERATURE_DEFAULT;
    QSet<QString> m_needSendSignalScreens;
    QTimer *m_wlComInitTimer = nullptr;
    QTimer *m_pSetTimer = nullptr;
    GmHelper *m_pGmHelper = nullptr;

};

#endif // GAMMAMANANGERTHREAD_H
