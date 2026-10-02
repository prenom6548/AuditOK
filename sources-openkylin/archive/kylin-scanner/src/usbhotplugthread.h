/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef USBHOTPLUGTHREAD_H
#define USBHOTPLUGTHREAD_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QStringList>
#include <sane/sane.h>
#include <QMessageBox>

class UsbHotplugThread : public QThread
{
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;

    bool exitWindowFlag = false;
    int hotplug_sock;
    QStringList USBdeviceList;
    const SANE_Device **g_deviceListusb = nullptr;
    void setExitFlagTrue();
    void setExitFlagFalse();
private:
    void initSane();
    void usbAddSlot(QString qstr);
    void usbRemoveSlot();
    bool usbDeviceIdentify(QString qstr);
    bool usbDeviceAdd(QString qstr);
    bool usbDeviceRemove(QString qstr);

    QStringList getConnectedScanner();
    QStringList getConnectedScanner2();

    QString getDeviceTypeFromPath(QString path);
    QString getRetFromCommand(QStringList command);
    void addToDeviceList(QString path);
    void removeToDeviceList(QString path);

    bool isRemoveScanner(QString path);

    QMessageBox::StandardButton box;

Q_SIGNALS:
    void usbRemove(QString);
    void usbAdd(QString);
};
#endif
