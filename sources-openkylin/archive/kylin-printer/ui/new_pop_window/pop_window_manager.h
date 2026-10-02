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
#ifndef POP_WINDOW_MANAGER_H
#define POP_WINDOW_MANAGER_H

#include <QObject>
#include "device_information.h"
#include <QQueue>


class PopWindowManager : public QObject
{
    Q_OBJECT
private:
    QQueue<DeviceInformation> m_infoList;
    QMap<QString, DeviceInformation> m_devMp;
    explicit PopWindowManager();
    ~PopWindowManager();
    void showPopWindow(const DeviceInformation &info);
    void getDevInfo(QString);
    void modifyInfoList(QString);
    void showNext();
    void addNewPrinterDevice(DeviceInformation devInfo);
    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk = -1;
    bool m_needInstall = false;
    bool m_pkgHelperFlag = false;
public:
    static PopWindowManager *getInstance();
signals:
    void installPrinter(QString);
    void manualInstallShowRequest(DeviceInformation &);
    void deviceViewShowRequest(QString &);
    void showNextRequest();
    void installEnd();
};




#endif // POP_WINDOW_MANAGER_H
