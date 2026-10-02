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
#ifndef GSETTINGSUBJECT_H
#define GSETTINGSUBJECT_H

#include <QObject>
#include <QGSettings>
#include "constant_class.h"
#include <QMutex>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
class GsettingSubject : public QObject
{
    Q_OBJECT
public:
    void iniWidgetStyle();
    void iniTimeZone();
    void iniFontSize();
    ~GsettingSubject();
    GsettingSubject(const GsettingSubject&)=delete;
    GsettingSubject& operator=(const GsettingSubject&)=delete;
    static GsettingSubject* getInstance(){
        static GsettingSubject instance;
        return &instance;
    }
    void iniMouseWheel();
    void iniScreenSaveState();
    bool getScreenSaveOn() const;
    bool getOnTablet() const;

signals:
    void blackStyle();
     void whiteStyle();
     void iconChnaged();
     void fontChanged(int size);
     void themeColorChanged();
     void mouseWheelChanged(int speed);
     void timeZoneChanged(QString timeZone);
     void screenSaveLock();
     void screenSaveUnlock();
private slots:
     void emitScreenSaveLock();
     void emitScreenSaveUnlock();
private:
     static void iniConnect();
     void iniConnection();
     void iniData();
     QGSettings *m_styleSettings = nullptr;
     QStringList m_stylelist ;
     QGSettings *m_formatSettings = nullptr;
     QGSettings *m_mouseSettings = nullptr;
     QDBusInterface *m_screenSaveInterface = nullptr;
     QDBusInterface *m_screenSaveIface = nullptr;
     bool onTablet = false;
     bool m_screenSaveOn = false;
     explicit GsettingSubject(QObject *parent = nullptr);
     void getWheelSpeed();
};

#endif // GSETTINGSUBJECT_H
