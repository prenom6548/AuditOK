/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "global_data.h"
#include "globalutils.h"

#include <QHostAddress>
#include <QNetworkInterface>
#include <QList>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusConnectionInterface>

#include <gsettingmonitor.h>

#include "view/common/globalsizedata.h"
#include "inisetting.h"

#define TCP_LISTEN_PORT 39901
#define UDP_LISTEN_PORT 39900

GlobalData::GlobalData()
{
    init();
}

GlobalData::~GlobalData() {}

GlobalData *GlobalData::getInstance(void)
{
    static GlobalData *s_globalData;
    if (s_globalData == NULL) {
        s_globalData = new GlobalData;
        s_globalData->setObjectName("kylin-ipmsg_globaldata_globalData");
    }

    return s_globalData;
}

void GlobalData::init(void)
{
    getTcpListenIp();
    getTcpListenPort();
    getUdpListenPort();
    getUuid();

    getFontName();
    changeFontName();
    changeFontSize();

    getDbusData();
    return;
}

/* 取第一个ipv4地址 , 进行监听 */
void GlobalData::getTcpListenIp(void)
{
    QList<QHostAddress> ipList = QNetworkInterface::allAddresses();

    for (int i = 0; i < ipList.count(); i++) {
        if (ipList.at(i) != QHostAddress::LocalHost && ipList.at(i).toIPv4Address()) {
            this->m_tcpListenIP = ipList.at(i).toString();
            break;
        }
    }
    return;
}

void GlobalData::getTcpListenPort(void)
{
    this->m_tcpListenPort = QString::number(TCP_LISTEN_PORT);

    return;
}

void GlobalData::getUdpListenPort(void)
{
    this->m_udpListenPort = QString::number(UDP_LISTEN_PORT);

    return;
}

void GlobalData::getUuid(void)
{
    this->m_uuid = IniSettings::getInstance()->getLocalUuid();

    return;
}

void GlobalData::getSystemTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    qInfo() << "system item : " << systemTheme;
    if (systemTheme == GlobalSizeData::BLACK_THEME) {
        GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUIDark;
    } else {
        GlobalSizeData::THEME_COLOR = GlobalSizeData::UKUILight;
    }

    return;
}

void GlobalData::getSystemFontSize()
{
    /* 获取系统初始字体 */
    double systemFontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    qInfo() << "system systemFontSize : " << systemFontSize;

    m_font12pxToPt = (systemFontSize * (12.00 / 11)) - 3;
    m_font14pxToPt = (systemFontSize * (14.00 / 11)) - 3;
    m_font16pxToPt = (systemFontSize * (16.00 / 11)) - 3;
    m_font18pxToPt = (systemFontSize * (18.00 / 11)) - 3;

    qDebug() << "Info : system init font : 12 ---> " << GlobalData::getInstance()->m_font12pxToPt << "14 ---> "
             << GlobalData::getInstance()->m_font14pxToPt << "16 ---> " << GlobalData::getInstance()->m_font16pxToPt
             << "18 ---> " << GlobalData::getInstance()->m_font16pxToPt;
    return;
}

void GlobalData::getSystemTransparency()
{
    double systemTransparency = kdk::GsettingMonitor::getSystemTransparency().toDouble();

    qInfo() << "system systemTransparency : " << systemTransparency;
    GlobalSizeData::BLUR_TRANSPARENCY = systemTransparency * 255;
    return;
}

QFont GlobalData::getFontSize12px()
{
    QFont font;
    font.setPointSizeF(m_font12pxToPt);

    return font;
}

QFont GlobalData::getFontSize14px()
{
    QFont font;
    font.setPointSizeF(m_font14pxToPt);

    return font;
}

QFont GlobalData::getFontSize16px()
{
    QFont font;
    font.setPointSizeF(m_font16pxToPt);

    return font;
}

QFont GlobalData::getFontSize18px()
{
    QFont font;
    font.setPointSizeF(m_font18pxToPt);

    return font;
}

void GlobalData::getFontName()
{
    if (QGSettings::isSchemaInstalled(THEME_GSETTINGS)) {
        /* 主题 */
        m_fontGsetting = new QGSettings(THEME_GSETTINGS, QByteArray(), this);
        m_fontGsetting->setObjectName("kylin-ipmsg_globaldata_QGSettings_fontGsetting");
        m_fontName = m_fontGsetting->get(APPLY_FONT_KEY).toString();
        m_colorName = m_fontGsetting->get(APPLY_HIGHT_LIGHT_COLOR).toString();
    }
    return;
}

void GlobalData::changeFontName()
{
    connect(m_fontGsetting, &QGSettings::changed, this, [=]() {
        if (m_fontGsetting->get(APPLY_FONT_KEY).toString() != m_fontName) {
            m_fontName = m_fontGsetting->get(APPLY_FONT_KEY).toString();
            emit signalChangeFont();
        }
        if (m_colorName != m_fontGsetting->get(APPLY_HIGHT_LIGHT_COLOR).toString()) {
            m_colorName = m_fontGsetting->get(APPLY_HIGHT_LIGHT_COLOR).toString();
            emit signalChangeHightColor();
        }
    });
    return;
}

void GlobalData::changeFontSize()
{
    this->connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        getSystemFontSize();
        emit signalChangeFontSize();
    });
    return;
}

void GlobalData::getDbusData()
{
    m_rotateChange = new RotateChangeInfo(this);
    connect(m_rotateChange, &RotateChangeInfo::sigRotationChanged, this, &GlobalData::slotChangeModel);
    getCurrentMode();

    return;
}

void GlobalData::slotChangeModel(bool isPCMode, int width, int height)
{
    m_currentSize.first = width;
    m_currentSize.second = height;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
        emit signalChangePC();
    } else {
        if (width > height) {
            m_currentMode = CurrentMode::HMode;
            emit signalChangeFlatH();
        } else {
            m_currentMode = CurrentMode::VMode;
            emit signalChangeFlatV();
        }
    }
    return;
}

CurrentMode GlobalData::getCurrentMode()
{
    bool isPCMode = m_rotateChange->getCurrentMode();
    QPair<int, int> currentSize = m_rotateChange->getCurrentScale();
    if (!m_rotateChange->m_existDbus) {
        m_currentMode = CurrentMode::PCMode;
        return m_currentMode;
    }
    m_currentSize = currentSize;
    if (isPCMode) {
        m_currentMode = CurrentMode::PCMode;
    } else {
        if (currentSize.first > currentSize.second) {
            m_currentMode = CurrentMode::HMode;
        } else {
            m_currentMode = CurrentMode::VMode;
        }
    }
    return m_currentMode;
}
