/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QFile>
#include <QDir>
#include <QFont>
#include <QFontMetrics>
#include <QTextOption>
#include <QPainter>
#include <QFile>
#include <QRegExp>
#include <QDebug>
#include <QScreen>
#include "definetypes.h"

#define CONFIG_FILE "/etc/lightdm/ukui-greeter.conf"
#define PROC_CPUINFO "/proc/cpuinfo"

Configuration *Configuration::m_instance = nullptr;

Configuration::Configuration(QObject *parent) : QObject(parent)
{
    QString recodfile = QDir::homePath() + "/.cache/ukui-greeter.conf";
    qDebug() << recodfile;

    configSettings = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    recordSettings = new QSettings(recodfile, QSettings::IniFormat, this);
    userSetting = new QSettings(configPath, QSettings::IniFormat, this);
}

Configuration *Configuration::instance(QObject *parent)
{
    if (m_instance == nullptr)
        m_instance = new Configuration(parent);
    return m_instance;
}

QVariant Configuration::getValue(const QString &key)
{
    configSettings->beginGroup("Greeter");
    QVariant value = configSettings->value(key);
    configSettings->endGroup();

    return value;
}

void Configuration::getCurrentUser(const QString userName)
{
    configPath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    userSetting = new QSettings(configPath, QSettings::IniFormat, this);
}

QVariant Configuration::getUserConfig(const QString &key)
{
    userSetting->beginGroup("Greeter");
    QVariant value = userSetting->value(key);
    userSetting->endGroup();
    qDebug() << " value = " << value;
    return value;
}

void Configuration::setValue(const QString &key, const QVariant &value)
{
    configSettings->beginGroup("Greeter");
    configSettings->setValue(key, value);
    configSettings->endGroup();
}

bool Configuration::hasValue(const QString &key)
{
    configSettings->beginGroup("Greeter");
    bool value = configSettings->contains(key);
    configSettings->endGroup();

    return value;
}

bool Configuration::getLastNumLock()
{
    recordSettings->beginGroup("Greeter");
    if (recordSettings->contains("numlock") == false) {
        recordSettings->setValue("numlock", true);
        recordSettings->sync();
        recordSettings->endGroup();
        return true;
    }
    bool lastNumLock = recordSettings->value("numlock").toBool();
    recordSettings->endGroup();
    return lastNumLock;
}

void Configuration::saveLastNumLock(bool value)
{
    recordSettings->beginGroup("Greeter");
    recordSettings->setValue("numlock", value);
    recordSettings->endGroup();
    recordSettings->sync();
}

bool Configuration::getIs990()
{
    if (hasCheck990) {
        return is990;
    }

    hasCheck990 = true;
    QRegExp r1("kirin.*9.0");
    r1.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp r2("pangu.*m900");
    r2.setCaseSensitivity(Qt::CaseInsensitive);

    QFile file(PROC_CPUINFO);
    if (!file.exists()) {
        is990 = false;
        return is990;
    }
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    is990 = (str.contains(r1) || str.contains(r2));
    file.close();

    if (is990)
        qDebug() << "is 990";

    return is990;
}

void Configuration::initShareConfig()
{
    QString userName = getenv("USER");
    QString configPath;
    configPath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    QFile fileConf(configPath);
    if (!fileConf.exists()) {
        QFile file(configPath);
        file.setPermissions(QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | QFile::ReadOther);
    }
    m_shareSettings = new QSettings(configPath, QSettings::IniFormat, this);
}

void Configuration::setShareConfigValue(const QString &key, const QVariant &value)
{
    if (!m_shareSettings)
        return;
    m_shareSettings->beginGroup("Greeter");
    m_shareSettings->setValue(key, value);
    m_shareSettings->endGroup();
    m_shareSettings->sync();
}

QString Configuration::getDefaultBackgroundName()
{
    return DEFAULT_BACKGROUND_PATH;
}

int Configuration::getRootBackgroundOption(QString userName)
{
    static QMap<QString, int> mapPicOptions = { { "scaled", 0 },    { "stretched", 1 }, { "centered", 2 },
                                                { "wallpaper", 3 }, { "zoom", 4 },      { "spanned", 5 } };
    QString userConfigurePath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    QFile backgroundFile(userConfigurePath);
    if (backgroundFile.exists()) {
        QSettings settings(userConfigurePath, QSettings::IniFormat);
        settings.beginGroup("greeter");
        if (settings.contains("picture-options")) {
            QString picOptions = settings.value("picture-options").toString();
            settings.endGroup();
            if (!picOptions.isEmpty() && mapPicOptions.contains(picOptions)) {
                return mapPicOptions[picOptions];
            }
        } else {
            settings.endGroup();
        }
    }
    return 0;
}
