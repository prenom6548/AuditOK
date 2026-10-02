/* configuration.h
 * Copyright (C) 2023 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QSettings>
#include <QPixmap>
#include <QGSettings>

class Configuration : public QObject
{
    Q_OBJECT
private:
    explicit Configuration(QObject *parent = nullptr);

public:
    static Configuration *instance(QObject *parent = nullptr);
    QVariant getValue(const QString &);
    QVariant getUserConfig(const QString &key);
    void setValue(const QString &, const QVariant &);
    bool hasValue(const QString &);

    void getCurrentUser(const QString userName);
    bool getLastNumLock();
    void saveLastNumLock(bool value);
    int getRootBackgroundOption(QString userName);
    bool getIs990();

    void initShareConfig();

    void setShareConfigValue(const QString &key, const QVariant &value);

    QString getDefaultBackgroundName();

    /**
     * @brief 获取是否是livecd模式
     * @return -1 未定义 0 不是livecd模式 1 livecd模式
     */
    static int checkIslivecdStatus();


    QString getAuthMethod();

    bool getProductNameN90F3();

private:
    QSettings *configSettings;
    QSettings *recordSettings;
    QSettings *userSetting;
    QString configPath;
    QGSettings *stylesettings = nullptr;
    bool hasCheck990 = false;
    bool is990 = false;
    QSettings *m_shareSettings = nullptr;
    static int m_livecdStatus;

    static Configuration *m_instance;
};

#endif // CONFIGURATION_H
