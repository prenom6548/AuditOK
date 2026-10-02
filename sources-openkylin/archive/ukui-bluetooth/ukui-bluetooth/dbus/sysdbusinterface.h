/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef SYSDBUSINTERFACE_H
#define SYSDBUSINTERFACE_H

#include <QStandardPaths>
#include <QObject>
#include <QtDBus/QDBusContext>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusConnectionInterface>

class sysdbusinterface : public QObject, protected QDBusContext
{
    Q_CLASSINFO("D-Bus Interface", "com.ukui.bluetooth")
    Q_OBJECT
public:
    sysdbusinterface();
    ~sysdbusinterface();

public slots:
    QString getProperFilePath(QString user, QString filename);

    void setMultiAudioCombine(bool v);
};

#endif // SYSDBUSINTERFACE_H
