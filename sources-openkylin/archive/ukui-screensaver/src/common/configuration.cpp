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
#include <unistd.h>
#include "definetypes.h"

#define CONFIG_FILE "/etc/lightdm/ukui-greeter.conf"
#define PROC_CPUINFO "/proc/cpuinfo"

Configuration *Configuration::m_instance = nullptr;
int Configuration::m_livecdStatus = -1;

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

static QString readFileSafely(const QString& filePath)
{
    QString strContent = "";
    if (!QFile::exists(filePath)) {
        qWarning() << "file not exists:" << filePath;
        return strContent;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "file open failed " << filePath << file.error();
        return strContent;
    }

    QTextStream testStream(&file);
    return testStream.readAll();
}

int Configuration::checkIslivecdStatus()
{
    if (m_livecdStatus != -1) {
        return m_livecdStatus;
    }
    QString strResult = readFileSafely("/proc/cmdline");
    if (strResult.contains("boot=casper")) {
        printf("is livecd\n");
        m_livecdStatus = 1;
        return m_livecdStatus;
    }

    QString filepath = QDir::homePath() + "/Desktop" + "/kylin-os-installer.desktop";
    QFileInfo file(filepath);
    if (!file.exists()) {
        m_livecdStatus = 0;
        return m_livecdStatus;
    }
    if (getuid() != 999) {
        m_livecdStatus = 0;
        return m_livecdStatus;
    }
    m_livecdStatus = 1;
    return m_livecdStatus;
}

// 这里临时用读取配置文件的方式替代原本调用dbus接口的方式，避免kylin-mfa的dbus接口
// 阻塞导致锁屏界面卡住的问题，后续kylin-mfa提供库函数的方式来获取双因子认证方法。
QString Configuration::getAuthMethod()
{
    QString configPath = "/etc/pwdless/default.conf";
    QFile file(configPath);
    if (!file.exists()) {
        return "";
    }
    QSettings settings(configPath, QSettings::IniFormat);
    // 检查[global]区块下mfaAuth
    QVariant mfaAuthVar = settings.value("global/mfaAuth", "false");
    if (mfaAuthVar.toString().toLower() != "true") {
        return "";
    }
    // 检查[mfa]区块下method
    QVariant methodVar = settings.value("mfa/method", "");
    QString method = methodVar.toString().trimmed();
    if (method.isEmpty()) {
        return "";
    }
    return method;
}

bool Configuration::getProductNameN90F3()
{
    QFile file("/sys/class/dmi/id/product_name");

    if (!file.exists()) {
        qWarning() << "File does not exist:" << file.fileName();
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << file.errorString();
        return false;
    }

    QString productName = file.readAll().trimmed();
    file.close();

    qDebug() << "Product name:" << productName;


    if (productName.contains("N90F3", Qt::CaseInsensitive) || productName.contains("N80F3", Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}
