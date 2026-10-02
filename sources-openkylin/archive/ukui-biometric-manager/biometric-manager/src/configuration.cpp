/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "configuration.h"
#include <QSettings>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDebug>

QString Configuration::configFile = QDir::homePath() + "/.biometric_auth/ukui_biometric.conf";

Configuration *Configuration::instance_ = new Configuration;

Configuration::Configuration(QObject *parent) : QObject(parent) {}

Configuration *Configuration::instance()
{
    return instance_;
}

QString Configuration::getDefaultDevice()
{
    QSettings settings(configFile, QSettings::IniFormat);

    return settings.value("DefaultDevice").toString();
}

void Configuration::setDefaultDevice(const QString &deviceName)
{
    // qDebug() << deviceName;
    QSettings settings(configFile, QSettings::IniFormat);

    settings.setValue("DefaultDevice", deviceName);
    settings.sync();

    // 由于greeter没有权限访问家目录，所以单独写一个配置文件
    QString configFile1 = QString("/var/lib/lightdm-data/%1/.biometric_auth/ukui_biometric.conf").arg(getenv("USER"));
    QSettings settings1(configFile1, QSettings::IniFormat);

    settings1.setValue("DefaultDevice", deviceName);
    settings1.sync();

    if (QString(getenv("USER")) == "root") {
        QSettings settings2("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        settings2.setValue("rootUserDefaultDevice", deviceName);
        settings2.sync();
    }
    emit defaultDeviceChanged(deviceName);
}

bool Configuration::getLoginSwitch()
{
    QSettings settings(configFile, QSettings::IniFormat);

    return settings.value("EnableGreeter").toBool();
}

void Configuration::setLoginSwitch(bool status)
{
    // qDebug() << status;
    QSettings settings(configFile, QSettings::IniFormat);

    settings.setValue("", status);
    settings.sync();

    // 由于greeter没有权限访问家目录，所以单独写一个配置文件
    QString configFile1 = QString("/var/lib/lightdm-data/%1/.biometric_auth/ukui_biometric.conf").arg(getenv("USER"));
    QSettings settings1(configFile1, QSettings::IniFormat);

    settings1.setValue("EnableGreeter", "true");
    settings1.sync();

    if (QString(getenv("USER")) == "root") {
        QSettings settings2("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        settings2.setValue("rootEnableGreeter", status);
        settings2.sync();
    }
}

bool Configuration::getSaverSwitch()
{
    QSettings settings(configFile, QSettings::IniFormat);

    return settings.value("EnableScreenster").toBool();
}

void Configuration::setSaverSwitch(bool status)
{
    // qDebug() << status;
    QSettings settings(configFile, QSettings::IniFormat);

    settings.setValue("", status);
    settings.sync();

    // 由于greeter没有权限访问家目录，所以单独写一个配置文件
    QString configFile1 = QString("/var/lib/lightdm-data/%1/.biometric_auth/ukui_biometric.conf").arg(getenv("USER"));
    QSettings settings1(configFile1, QSettings::IniFormat);

    settings1.setValue("EnableScreenster", "true");
    settings1.sync();

    if (QString(getenv("USER")) == "root") {
        QSettings settings2("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        settings2.setValue("rootEnableScreenster", "true");
        settings2.sync();
    }
}

bool Configuration::getSystemSwitch()
{
    QSettings settings(configFile, QSettings::IniFormat);

    return settings.value("EnablePolkit").toBool();
}

void Configuration::setSystemSwitch(bool status)
{
    // qDebug() << status;
    QSettings settings(configFile, QSettings::IniFormat);

    settings.setValue("", status);
    settings.sync();

    // 由于greeter没有权限访问家目录，所以单独写一个配置文件
    QString configFile1 = QString("/var/lib/lightdm-data/%1/.biometric_auth/ukui_biometric.conf").arg(getenv("USER"));
    QSettings settings1(configFile1, QSettings::IniFormat);

    settings1.setValue("EnablePolkit", "true");
    settings1.sync();

    if (QString(getenv("USER")) == "root") {
        QSettings settings2("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        settings2.setValue("rootEnablePolkit", "true");
        settings2.sync();
    }
}

QString Configuration::get_current_version()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = "1.0.1-1kylin1~43";

    pp = popen("dpkg -l  ukui-biometric-manager | grep  ukui-biometric-manager", "r");
    if (NULL == pp)
        return version;

    while ((read = getline(&line, &len, pp)) != -1) {
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    pclose(pp);
    return version;
}

// 获取两个字符串的开头的相同部分
QString Configuration::get_same_str_start(QString &str1, QString &str2)
{
    int size = std::min(str1.size(), str2.size());
    QByteArray ba = str1.toLatin1();
    const char *p1 = ba.data();

    QByteArray ba2 = str2.toLatin1();
    const char *p2 = ba2.data();

    int stopAt = 0;
    for (int i = 0; i < size; ++i) {
        if (*p1 != *p2) {
            break;
        }
        ++p1;
        ++p2;
        ++stopAt;
    }
    QString sameStr;
    if (stopAt == 0)
        return "";
    else {
        sameStr = str1.left(stopAt);
        // 判断当人脸1与人脸10的情况，即相同字符串为‘人脸1’
        for (int i = sameStr.length() - 1; i >= 0; i--) {
            if (isNum(sameStr.mid(i, 1))) {
                sameStr = sameStr.left(i);
            }
        }
    }
    return sameStr;
}

// 判断一个字符串是否为数字
bool Configuration::isNum(QString str)
{
    bool isNum;
    str.toDouble(&isNum);
    return isNum;
}
