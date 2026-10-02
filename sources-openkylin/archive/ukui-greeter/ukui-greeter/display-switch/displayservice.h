/* displayservice.h
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
#ifndef DISPLAYSERVICE_H
#define DISPLAYSERVICE_H

#include <QObject>
#include <QVector>
#include <QMap>

enum DisplayMode{
    DISPLAY_MODE_ORI        = 1,
    DISPLAY_MODE_CLONE      = 2,
    DISPLAY_MODE_EXTEND     = 3,
    DISPLAY_MODE_ONLY_OUT   = 4
};

class DisplayService : public QObject
{
    Q_OBJECT
private:
    explicit DisplayService(QObject *parent = nullptr);

public:
    static DisplayService *instance(QObject *parent = nullptr);
    bool switchDisplayMode(DisplayMode targetMode);
    void setOneDisplayMode();//单屏模式设置
    void setCurUserName(QString strUserName);
    bool isSaveParamInUsed();
    int isJJW7200();

private:
    void getMonitors();

    QMap<QString, QVector<QString>> monitors;
    QStringList monitorNames;
    QString m_strSaveParamPath = "";
    QString m_strUserName = "";
    static DisplayService  *m_instance;
};

#endif // DISPLAYSERVICE_H
