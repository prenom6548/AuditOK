/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef LOGIN1HELPER_H
#define LOGIN1HELPER_H

#include <QDBusConnection>
#include <QVariantMap>
#include <QObject>
#include "../common/definetypes.h"

/**
 * @brief login1访问工具类
 *
 */
class Login1Helper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit Login1Helper(QObject *parent = nullptr);
    /**
     * @brief 析构
     *
     */
    virtual ~Login1Helper();

    /**
     * @brief 当前会话是否处于活跃
     *
     * @return bool true 是，否则否
     */
    bool isSessionActive();

    void setPowerManager(const QString &powerManagerfunc);

    bool getCanPowerManager(const QString &powerManagerfunc);
    bool isCanHibernate();
    bool isCanSuspend();
    bool isCanReboot();
    bool isCanPowerOff();
    bool canAction(Action action);

    void Unlock();

public Q_SLOTS:
    /**
     * @brief 会话属性改变
     *
     * @param QString 接口名
     * @param QVariantMap 变化的属性表
     * @param QStringList 无效的属性
     */
    void onSessionPropChanged(QString, QVariantMap, QStringList);
    /**
     * @brief 准备休眠/唤醒
     *
     * @param isSleep true 休眠，false 唤醒
     */
    void onPrepareForSleep(bool isSleep);
    /**
     * @brief 程序阻塞关机/睡眠状态改变
     *
     */
    void onHibitedWatcherMessage(void);

Q_SIGNALS:
    /**
     * @brief 请求锁定
     *
     */
    void requestLock();
    /**
     * @brief 请求解锁
     *
     */
    void requestUnlock();
    /**
     * @brief 会话活跃状态改变
     *
     * @param isActive true 活跃，false 不活跃
     */
    void sessionActiveChanged(bool isActive);
    /**
     * @brief 准备休眠/唤醒
     *
     * @param isSleep true 休眠，false 唤醒
     */
    void PrepareForSleep(bool isSleep);
    /**
     * @brief 程序阻塞关机/睡眠状态改变
     *
     * @param blockInhibited 阻塞类型
     */
    void blockInhibitedChanged(QString blockInhibited);

private:
    bool m_isSessionActive = false; /**< 是否处于活跃状态 */
    bool m_isSleeping = false;      /**< 是否处于休眠状态 */

    bool m_isCanLockScreen = true;
    bool m_isCanLogout = true;

    QString m_strSessionPath; // 当前会话路径
    QDBusInterface *m_login1Interface = nullptr;
};

#endif // LOGIN1HELPER_H
