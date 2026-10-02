/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#include <QApplication>
#include "globalv.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#include "glibinterface.h"

#define CONFIG_FILE "/etc/lightdm/ukui-greeter.conf"
#define PROC_CPUINFO "/proc/cpuinfo"

Configuration* Configuration::instance_ = nullptr;

Configuration::Configuration(QObject *parent)
    : QObject(parent),
      m_trans(nullptr)
{
    QString recodfile = QDir::homePath() + "/.cache/ukui-greeter.conf";
    qDebug() << recodfile;

    configSettings = new QSettings(CONFIG_FILE, QSettings::IniFormat, this);
    recordSettings = new QSettings(recodfile, QSettings::IniFormat, this);
    userSetting = new QSettings(configPath, QSettings::IniFormat, this);
}

Configuration* Configuration::instance(QObject *parent)
{
    if(instance_ == nullptr)
        instance_ = new Configuration(parent);
    return instance_;
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
    qDebug() << " value = " << value ;
    return value;
}

void Configuration::setUsersFont()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        if (!stylesettings) {
            stylesettings = new QGSettings("org.ukui.style","",this);
        }
        double defaultFontSize = getDefaultFontSize();
        int fontSize = getUserConfig("fontSize").toDouble() + defaultFontSize;
        stylesettings->set("systemFontSize", fontSize);
    }
}

void Configuration::setUsersThemeColor()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        if (!stylesettings) {
            stylesettings = new QGSettings("org.ukui.style","",this);
        }
        QString themeColor = getUserConfig("themeColor").toString();
        stylesettings->set("themeColor", themeColor);
    }
}

double Configuration::getPtToPx()
{
    double m_ptTopx = 1.0;
    if (QApplication::primaryScreen()->logicalDotsPerInch() > 0)
        m_ptTopx = 72/(QApplication::primaryScreen()->logicalDotsPerInch());
    return m_ptTopx;
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

QString Configuration::getLastLoginUser()
{
    recordSettings->beginGroup("Greeter");
    QString lastLoginUser = recordSettings->value("lastLoginUser").toString();
    recordSettings->endGroup();
    return lastLoginUser;
}

void Configuration::saveLastLoginUser(const QString &userRealName)
{
    recordSettings->beginGroup("Greeter");
    recordSettings->setValue("lastLoginUser", userRealName);
    recordSettings->endGroup();
    recordSettings->sync();
}

void Configuration::saveLastLoginUser1(const QString &userRealName)
{
    recordSettings->beginGroup("Greeter");
    recordSettings->setValue("lastLoginUser1", userRealName);
    recordSettings->endGroup();
    recordSettings->sync();
}

bool Configuration::getLastNumLock()
{
    recordSettings->beginGroup("Greeter");
    if(recordSettings->contains("numlock") == false){
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

/**
 * 获取系统版本号
 */
QString getSystemVersion()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString release = settings.value("DISTRIB_RELEASE").toString();
    QString description = settings.value("DISTRIB_DESCRIPTION").toString();
    if(description.right(3) == "LTS")
        release = release + " LTS";
    return release;
}

QString getSystemDistrib()
{
    QSettings settings("/etc/lsb-release", QSettings::IniFormat);
    QString distribId = settings.value("DISTRIB_ID").toString();
    return distribId;
}

bool Configuration::getUseFirstDevice()
{
    QSettings settings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
    return settings.value("UseFirstDevice").toBool();
}

/**
 * @brief logoGenerator
 * @param text
 * @return
 * 生成logo
 */
QPixmap logoGenerator(const QString &text)
{
//    if(getSystemDistrib() == "Kylin")
//        return QPixmap(":/images/kylin-logo.png");

//    QString logoFile(IMAGE_DIR + "logo.png");
//    if(QFile(logoFile).exists())
//    {
//        QPixmap logo(IMAGE_DIR + "logo.png");
//        return logo;
//    }
    QPixmap logoBare;
    logoBare.load(":/images/uk-logo.png");

    QFont font("ubuntu", 18);
    QFontMetrics fm(font);
    int textPixelSize = fm.width(text);

    QPixmap logo(logoBare.width() + textPixelSize + 3 + 30, logoBare.height());
    logo.fill(Qt::transparent);

    QRect logoBareRect(30 , 0, logoBare.width(), logoBare.height());
    QPainter painter;
    painter.begin(&logo);
    painter.drawPixmap(logoBareRect, logoBare, logoBare.rect());

    painter.setPen(Qt::white);
    painter.setFont(font);
    QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::WordWrap);
    QRect versionRect(logoBareRect.right()+3, 3, logo.width() - logoBare.width(), logo.height());
    painter.drawText(versionRect, text, option);

    return logo;
}


QPixmap Configuration::getLogo()
{
    /**
     * 先从配置文件中读取logo文件，如果不存在，则生成logo图片
     */
    QString logoFilePath = getValue("logo").toString();
    if(QFile(logoFilePath).exists())
        return QPixmap(logoFilePath);

    return logoGenerator(getSystemVersion());
}


QString Configuration::getDefaultBackgroundName()
{
    char *systemName = kdk_system_get_systemName();
    if (systemName) {
        if (QString(systemName) == QString("openKylin")) {
            free(systemName);
            return "/usr/share/backgrounds/1-openkylin.jpg";
        }
        free(systemName);
    }
    return "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
}

int Configuration::getRootBackgroundOption(QString userName)
{
    static QMap<QString, int> mapPicOptions = {{"scaled",0},{"stretched",1},{"centered",2},{"wallpaper",3},{"zoom",4},{"spanned",5}};
    QString userConfigurePath = QString("/var/lib/lightdm-data/%1/ukui-greeter.conf").arg(userName);
    QFile backgroundFile(userConfigurePath);
    if(backgroundFile.exists()){
        QSettings settings(userConfigurePath,QSettings::IniFormat);
        settings.beginGroup("greeter");
        if(settings.contains("picture-options")){
            QString picOptions = settings.value("picture-options").toString();
            if(!picOptions.isEmpty() && mapPicOptions.contains(picOptions)){
                return mapPicOptions[picOptions];
            }
        }
    }
    return 0;
}

bool Configuration::getIs990()
{
    if(hasCheck990){
        return is990;
    }

    hasCheck990 = true;
    QRegExp  r1("kirin.*9.0");
    r1.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp r2("pangu.*m900");
    r2.setCaseSensitivity(Qt::CaseInsensitive);

    QFile file(PROC_CPUINFO);
    if(!file.exists()){
        is990 = false;
        return is990;
    }
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    is990 = (str.contains(r1) || str.contains(r2));
    file.close();

    if(is990)
        qDebug()<<"is 990";

    return is990;
}
