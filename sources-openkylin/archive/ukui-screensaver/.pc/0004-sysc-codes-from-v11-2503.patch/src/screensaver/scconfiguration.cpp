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
#include "scconfiguration.h"
#include "utils.h"
#include <QDebug>
#include <QSettings>
#include <QGSettings>
#include <QString>
#include <QScreen>
#include <QApplication>
#include "lock-dialog/backenddbushelper.h"
#include "definetypes.h"

SCConfiguration *SCConfiguration::instance_ = nullptr;

SCConfiguration::SCConfiguration(QObject *parent) : QObject(parent)
{
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
    m_helperBackendDbus = new BackendDbusHelper(sessionDbus, SS_DBUS_PATH, QDBusConnection::sessionBus(), this);
    if (!m_helperBackendDbus->isValid()) {
        delete m_helperBackendDbus;
        m_helperBackendDbus = new BackendDbusHelper(SS_DBUS_SERVICE, SS_DBUS_PATH, QDBusConnection::sessionBus(), this);
    }
    connect(
        m_helperBackendDbus,
        &BackendDbusHelper::screenSaverConfChanged,
        this,
        &SCConfiguration::onConfigurationChanged);
    connect(
        m_helperBackendDbus,
        &BackendDbusHelper::ukccPluginsConfChanged,
        this,
        &SCConfiguration::onConfigurationChanged);
    connect(
        m_helperBackendDbus, &BackendDbusHelper::themeStyleConfChanged, this, &SCConfiguration::onConfigurationChanged);
    connect(
        m_helperBackendDbus, &BackendDbusHelper::lockScreenConfChanged, this, &SCConfiguration::onConfigurationChanged);
}

SCConfiguration *SCConfiguration::instance(QObject *parent)
{
    if (instance_ == nullptr)
        instance_ = new SCConfiguration(parent);
    return instance_;
}

void SCConfiguration::onConfigurationChanged(QString key, QVariant value)
{
    if (key == "cycleTime") {
        int cycleTime = value.toInt();
        Q_EMIT cycleTimeChanged(cycleTime);
    } else if (key == "automaticSwitchingEnabled") {
        bool changed = value.toBool();
        Q_EMIT autoSwitchChanged(changed);
    } else if (key == "backgroundPath") {
        QString path = value.toString();
        Q_EMIT backgroundPathChanged(path);
    } else if (key == "background") {
        QString path = value.toString();
        Q_EMIT backgroundChanged(path);
    } else if (key == "mytext") {
        QString text = value.toString();
        Q_EMIT myTextChanged(text);
    } else if (key == "showCustomRestTime") {
        bool ret = value.toInt();
        Q_EMIT showCRestTimeChanged(ret);
    } else if (key == "showUkuiRestTime") {
        bool ret = value.toInt();
        Q_EMIT showURestTimeChanged(ret);
    } else if (key == "textIsCenter") {
        bool ret = value.toBool();
        Q_EMIT textIsCenterChanged(ret);
    } else if (key == "showMessageEnabled") {
        bool ret = value.toBool();
        Q_EMIT messageShowEnableChanged(ret);
    } else if (key == "messageNumber") {
        int num = value.toInt();
        Q_EMIT messageNumberChanged(num);
    } else if (key == "hoursystem") {
        int timeType = value.toInt();
        Q_EMIT timeTypeChanged(timeType);
    } else if (key == "type") {
        QString dateType = value.toString();
        Q_EMIT dateTypeChanged(dateType);
    } else if (key == "menuTransparency") {
        int blur_Num = value.toInt();
        Q_EMIT blurChanged(blur_Num);
    } else if (key == "styleName") {
        QString m_curStyle = value.toString();
        Q_EMIT styleChanged(m_curStyle);
    } else if (key == "systemFontSize") {
        double m_curFontSize = value.toDouble();
        qDebug() << "curFontSize = " << m_curFontSize;
        Q_EMIT fontSizeChanged(m_curFontSize);
    }
}

QString SCConfiguration::getDefaultBackground()
{
    QString backgroundFile = m_helperBackendDbus->getLockScreenConf(KEY_BACKGROUND).toString();
    if (ispicture(backgroundFile))
        return backgroundFile;
    else
        return DEFAULT_BACKGROUND_PATH;
}

int SCConfiguration::getTimeType()
{
    int timeType = 24;
    timeType = m_helperBackendDbus->getUkccPluginsConf(KEY_HOUR_SYSTEM).toInt();
    return timeType;
}

QString SCConfiguration::getDateType()
{
    QString dateType = "cn";
    dateType = m_helperBackendDbus->getUkccPluginsConf(KEY_DATE).toString();
    return dateType;
}

int SCConfiguration::getFontSize()
{
    double fontSize = 0;
    fontSize = m_helperBackendDbus->getThemeStyleConf(KEY_SYSTEM_FONT_SIZE).toDouble();
    double defaultFontSize = getDefaultFontSize();
    qDebug() << "defaultFontSize = " << defaultFontSize;

    return fontSize - defaultFontSize;
}

double SCConfiguration::getPtToPx()
{
    double m_ptTopx = 1.0;
    if (QApplication::primaryScreen()->logicalDotsPerInch() > 0)
        m_ptTopx = 72 / (QApplication::primaryScreen()->logicalDotsPerInch());
    return m_ptTopx;
}

bool SCConfiguration::getAutoSwitch()
{
    bool ret = false;
    ret = m_helperBackendDbus->getScreenSaverConf(KEY_AUTOMATIC_SWITCHING_ENABLE).toBool();
    return ret;
}

bool SCConfiguration::getIsCustom()
{
    bool ret = false;
    ret = (m_helperBackendDbus->getLockScreenConf("mode").toString() == "default-ukui-custom");

    return ret;
}

bool SCConfiguration::getMessageShowEnable()
{
    bool ret = false;
    ret = m_helperBackendDbus->getScreenSaverConf(KEY_SHOW_MESSAGE_ENABLED).toBool();

    return ret;
}

int SCConfiguration::getMessageNumber()
{
    bool ret = false;
    ret = (m_helperBackendDbus->getLockScreenConf("mode").toString() == "default-ukui-custom");

    return ret;
}

bool SCConfiguration::getCShowRestTime()
{
    bool ret = true;
    ret = m_helperBackendDbus->getScreenSaverConf(KEY_SHOW_CUSTOM_REST_TIME).toBool();
    return ret;
}

bool SCConfiguration::getUShowRestTime()
{
    bool ret = true;
    ret = m_helperBackendDbus->getScreenSaverConf(KEY_SHOW_UKUI_REST_TIME).toBool();
    return ret;
}

int SCConfiguration::getCycleTime()
{
    int cycleTime = 300;
    cycleTime = m_helperBackendDbus->getScreenSaverConf(KEY_CYCLE_TIME).toInt();
    return cycleTime;
}

QString SCConfiguration::getBackgroundPath()
{
    QString backgroundPath = "/usr/share/backgrounds";
    backgroundPath = m_helperBackendDbus->getScreenSaverConf(KEY_BACKGROUND_PATH).toString();
    return backgroundPath;
}

bool SCConfiguration::getTextIsCenter()
{
    bool ret = true;
    ret = m_helperBackendDbus->getScreenSaverConf(KEY_TEXT_IS_CENTER).toBool();
    return ret;
}

QString SCConfiguration::getMyText()
{
    QString myText = "";
    myText = m_helperBackendDbus->getScreenSaverConf("mytext").toString();
    return myText;
}

int SCConfiguration::getBlurNumber()
{
    int blurNum = 50;
    blurNum = m_helperBackendDbus->getThemeStyleConf("menuTransparency").toInt();
    return blurNum;
}

QString SCConfiguration::getcurStyle()
{
    QString curStyle;
    curStyle = m_helperBackendDbus->getThemeStyleConf("styleName").toString();
    return curStyle;
}

QString SCConfiguration::getVideoPath()
{
    QString videoPath;
    videoPath = m_helperBackendDbus->getScreenSaverConf(KEY_VIDEO_PATH).toString();
    return videoPath;
}

QString SCConfiguration::getVideoFormat()
{
    QString videoFormat;
    videoFormat = m_helperBackendDbus->getScreenSaverConf(KEY_VIDEO_FORMAT).toString();
    return videoFormat;
}

int SCConfiguration::getVideoSize()
{
    int videoSize;
    videoSize = m_helperBackendDbus->getScreenSaverConf(KEY_VIDEO_SIZE).toInt();
    return videoSize;
}

int SCConfiguration::getVideoWidth()
{
    int videoWidth;
    videoWidth = m_helperBackendDbus->getScreenSaverConf(KEY_VIDEO_WIDTH).toInt();
    return videoWidth;
}

int SCConfiguration::getVideoHeight()
{
    int videoHeight;
    videoHeight = m_helperBackendDbus->getScreenSaverConf(KEY_VIDEO_HEIGHT).toInt();
    return videoHeight;
}
