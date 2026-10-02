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
#ifndef UPOWERHELPER_H
#define UPOWERHELPER_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariantMap>
#include <QObject>

/**
 * @brief login1访问工具类
 *
 */
class UpowerHelper : public QObject
{
    Q_OBJECT
public:
    explicit UpowerHelper(QObject *parent = nullptr);

    virtual ~UpowerHelper();
    void init();

    QStringList getBatteryArgs();

    QString getBatteryIconName();

    inline bool getIsBattery()
    {
        return m_isBattery;
    }

private Q_SLOTS:
    void onUpdateBatteryInfo();

    void dealAcOnlineStateChanged(int, bool);
    void dealBatteryInfoChanged(int, QStringList);
    void dealDeviceChanged();


    /**
     * @brief onLidStateChanged 屏幕开关信号监听
     */
    void onLidStateChanged(bool value);
    /**
     * @brief onServiceStatusChanged upower服务状态变化
     * @param strName 服务名称
     * @param isActive 状态
     */
    void onServiceStatusChanged(QString strName, bool isActive);

Q_SIGNALS:
    /**
     * @brief upowerServiceReady upower服务已准备好
     */
    void upowerServiceReady();
    /**
     * @brief batteryStatusChanged 电池状态变化
     * @param iconName 图标名称
     */
    void batteryStatusChanged(QString iconName);
    /**
     * @brief batteryChanged 电池状态改变
     * @param args 参数
     */
    void batteryChanged(QStringList args);

    /**
     * @brief lidStateChanged 屏幕亮屏状态
     * @param isClosed 是否关屏
     */
    void lidStateChanged(bool isClosed);

private:
    /**
     * @brief isDesktop 是否是台式机
     * @return true 是，其他 否
     */
    bool isDesktop();

private:
    QDBusInterface *m_upowerInterface = nullptr;
    QDBusInterface *m_batInterface = nullptr;
    QDBusInterface *m_upowerService = nullptr;

    bool m_isBattery = false;
    bool m_isInited = false;
    bool m_serviceIsRuning = false;
};

#endif // UPOWERHELPER_H
