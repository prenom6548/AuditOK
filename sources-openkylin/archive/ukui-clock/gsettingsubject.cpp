/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "gsettingsubject.h"
#include "theme.h"


GsettingSubject::GsettingSubject(QObject *parent) : QObject(parent)
{
    iniData();
    iniConnection();
}

void GsettingSubject::iniConnection()
{
    if(m_styleSettings!=nullptr){
        connect(m_styleSettings, &QGSettings::changed, this, [=] (const QString &key){
            if(key==STYLE_NAME){
                if(m_stylelist.contains(m_styleSettings->get(STYLE_NAME).toString())){
                    theme::themetype=1;
                    emit blackStyle();
                }else{
                    theme::themetype=0;
                    emit whiteStyle();
                }
            }
            if(key==STYLE_ICON_NAME || key==STYLE_ICON){
               emit iconChnaged();
            }
            if (key == SYSTEM_FONT_SIZE) {
                //现在控制面板的字体有：10 12 13.5 15，fontChanged释放信号字体：10 12 13 15
                const double size = m_styleSettings->get(SYSTEM_FONT_EKY).toDouble();
                emit fontChanged(size);
            }
            if (key==THEME_COLOR) {
                emit themeColorChanged();
            }
        });
    }
    if(m_formatSettings!=nullptr){
        connect(m_formatSettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == HOUR_SYSTEM) {
                QString m_timeZone = m_formatSettings->get(TIME_FORMAT_KEY).toString();
                emit timeZoneChanged(m_timeZone);
            }
        });
    }
    if(m_mouseSettings!=nullptr){
        connect(m_mouseSettings, &QGSettings::changed, this, [=] (const QString &key) {
            if(WHEEL_KEY_HW==key||WHEEL_KEY_SP==key){
                getWheelSpeed();
            }
        });
    }
    if (m_screenSaveInterface->isValid()){
        QObject::connect(m_screenSaveInterface, SIGNAL(lock()),
                         this,  SLOT(emitScreenSaveLock()));
    }
    if (m_screenSaveInterface->isValid()){
        QObject::connect(m_screenSaveInterface, SIGNAL(unlock()),
                         this,  SLOT(emitScreenSaveUnlock()));
    }

}

void GsettingSubject::iniData()
{
    const QByteArray style_id(ORG_UKUI_STYLE);
    m_stylelist<<STYLE_NAME_KEY_DARK<<STYLE_NAME_KEY_BLACK;
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
    const QByteArray timeId(FORMAT_SCHEMA);
    if (QGSettings::isSchemaInstalled(timeId)){
        m_formatSettings = new QGSettings(timeId);
    }
    const QByteArray mouseId(MOUSE_SCHEMA);
    if (QGSettings::isSchemaInstalled(mouseId)){
        m_mouseSettings = new QGSettings(mouseId);
    }
    //锁屏接口
    m_screenSaveInterface = new QDBusInterface(SCREENSAVE_DBUS_SERVICE, SCREENSAVE_DBUS_PATH,
                                                  SCREENSAVE_DBUS_INTERFACE,
                                                  QDBusConnection::sessionBus());
    if (!m_screenSaveInterface->isValid()){
        qCritical() << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }
}

bool GsettingSubject::getScreenSaveOn() const
{
    return m_screenSaveOn;
}
bool GsettingSubject::getOnTablet() const
{
    return onTablet;
}

/**
 * @brief 初始化主题
 */
void GsettingSubject::iniWidgetStyle()
{
    if(m_styleSettings!=nullptr){
        if(m_stylelist.contains(m_styleSettings->get(STYLE_NAME).toString())){
            emit blackStyle();
        }else{
            emit whiteStyle();
        }
    }

}
/**
 * @brief 初始化时区
 */
void GsettingSubject::iniTimeZone()
{
    // 监听时区变化
    if(m_formatSettings!=nullptr){
        QString m_timeZone = m_formatSettings->get(TIME_FORMAT_KEY).toString();
        emit timeZoneChanged(m_timeZone);
    }

}
/**
 * @brief 初始化字体
 */
void GsettingSubject::iniFontSize()
{
    if(m_styleSettings!=nullptr){
        //字体
        if (m_styleSettings->get(SYSTEM_FONT_EKY).toInt()) {
            const double size = m_styleSettings->get(SYSTEM_FONT_EKY).toDouble();
            emit fontChanged(size);
        }
    }
}
void GsettingSubject::iniMouseWheel()
{
    if(m_mouseSettings!=nullptr){
        getWheelSpeed();
    }
}

void GsettingSubject::iniScreenSaveState()
{
    m_screenSaveIface = new QDBusInterface(SSWND_DBUS_SERVICE,
                                      SSWND_DBUS_PATH,
                                      SSWND_DBUS_INTERFACE);
    if ( m_screenSaveIface&& m_screenSaveIface->isValid()) {
        emit screenSaveLock();
    }else{
        emit screenSaveUnlock();
    }

}
void GsettingSubject::getWheelSpeed()
{
    if (m_mouseSettings->get(WHEEL_KEY_HW).toInt()) {
        const int speed = m_mouseSettings->get(WHEEL_KEY_HW).toInt();
        emit mouseWheelChanged(speed);
    }
    if (m_mouseSettings->get(WHEEL_KEY_SP).toInt()) {
        const int speed = m_mouseSettings->get(WHEEL_KEY_SP).toInt();
        emit mouseWheelChanged(speed);
    }
}

void GsettingSubject::emitScreenSaveLock()
{
    qDebug()<<"dbq-emitScreenSaveLock";
    m_screenSaveOn = true;
    emit screenSaveLock();
}

void GsettingSubject::emitScreenSaveUnlock()
{
    qDebug()<<"dbq-emitScreenSaveUnlock";
    m_screenSaveOn = false;
    emit screenSaveUnlock();
}

GsettingSubject::~GsettingSubject()
{
    delete m_styleSettings;
    delete m_formatSettings;
    delete m_mouseSettings;
}
