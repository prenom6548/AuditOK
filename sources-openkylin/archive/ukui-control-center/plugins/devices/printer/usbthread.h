/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef USBTHREAD_H
#define USBTHREAD_H



#include <QObject>

class UsbThread : public QObject
{
    Q_OBJECT
public:
    explicit UsbThread();
    ~UsbThread();

    void run();

private:
    void usbDeviceIdentify(const QString &str);
    int init_sock();
    QString getDeivceTypeFromPath(const QString &path);
    QString getRetFromCommand(const QStringList &command);

signals:
    void addsignal();
    void removesignal();
};

#endif // USBTHREAD_H
