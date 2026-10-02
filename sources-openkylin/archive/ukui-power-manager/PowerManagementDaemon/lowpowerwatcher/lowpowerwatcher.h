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


#ifndef LOWPOWERWATCHER_H
#define LOWPOWERWATCHER_H
#include <QTimer>

#include "acwatcher/acwatcher.h"
#include "eventwatcher.h"

class LowPowerWatcher : public EventWatcher
{
    Q_OBJECT
public:
    LowPowerWatcher();
    ~LowPowerWatcher();

    /**
     * @brief initLowPowerWatcher
     * 初始化LowPowerWatcher
     */
    void initLowPowerWatcher();

    bool lowPowerState();

private:
    /**
     * @brief initConnect
     * 初始化连接
     */
    void initConnect();

    /**
     * @brief readSettings
     * 读取gsetting设置
     */
    void readSettings();

    /**
     * @brief readPercentage
     * 从UPower读取当前电量百分比
     * @return
     */
    double readPercentage();

    /**
     * @brief lowBatteryNotify
     * 低电量消息通知
     */
    void lowBatteryNotify();

    /**
     * @brief veryLowBatteryNotify
     * @param msg
     * 极低电量执行操作通知
     */
    void veryLowBatteryNotify(const QString &msg);

    /**
     * @brief veryLowBatteryContrl
     * 极低电量进行操作
     */
    void veryLowBatteryContrl(const QString &contrl);

    /**
     * @brief veryLowBatteryContrlStop
     * 极低电量操作计时器打断
     */
    void veryLowBatteryContrlStop();

    quint32 notifySend(const QString &type, const QString &arg, QString soundName);

    /**
     * @brief mPercentageLow
     * 当前电量百分比
     */
    double mPercentageLow;

    /**
     * @brief mSetPercentageLow
     * 用户设置的低电量百分比
     */
    int mSetPercentageLow;

    /**
     * @brief mPercentageAction
     * 用户设置的极低电量百分比
     */
    int mPercentageAction;

    /**
     * @brief mActionCriticalBattery
     * 极低电量进行的操作设置
     */
    QString mActionCriticalBattery;

    //    AcWatcher *mAcWatcher;

    QTimer *mContrlTimer;

    bool mLowPowerState;

    bool mNotifyState;

    quint32 m_notificationId = 0;
    quint32 m_criticalBatteryNotificationId = 0;

    // signals:
    //    /**
    //     * @brief lowPowerChanged
    //     * 进入低电量状态信号
    //     */
    //    void lowPowerChanged(bool);

    //降低亮度
    void reduceBrightness();
    //增加亮度
    void increaseBrightness();

    void closeNotify(quint32 notificationId);

private slots:

    void dealPowerStateMssage(bool state);

    void dealLowBatteryMssage(bool state);

    void dealVeryLowBatteryMssage(bool state);

signals:
    void percentageLowSignal(bool);
};
#endif // LOWPOWERWATCHER_H
