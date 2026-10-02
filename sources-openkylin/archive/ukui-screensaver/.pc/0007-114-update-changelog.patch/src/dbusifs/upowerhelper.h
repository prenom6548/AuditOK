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
#include <QVariantMap>
#include <QObject>

class EngineDevice;

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

    QStringList getBatteryArgs();

    QString getBatteryIconName();

    inline bool getIsBattery()
    {
        return m_isBattery;
    }

private Q_SLOTS:

    void onBatteryChanged(QStringList args);

    void dealMessage(QDBusMessage);

    /**
     * @brief onLidWatcherMessage 屏幕开关信号监听
     */
    void onLidWatcherMessage();

Q_SIGNALS:
    void batteryStatusChanged(QString iconName);

    void batteryChanged(QStringList args);

    /**
     * @brief lidStateChanged 屏幕亮屏状态
     * @param isClosed 是否关屏
     */
    void lidStateChanged(bool isClosed);

private:
    QDBusInterface *m_upowerInterface = nullptr;
    QDBusInterface *m_batInterface = nullptr;
    QDBusInterface *m_upowerService = nullptr;

    EngineDevice *m_engineDevice = nullptr;

    bool m_isBattery = false;
};

#endif // UPOWERHELPER_H
