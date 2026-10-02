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
#ifndef BIODEVICES_H
#define BIODEVICES_H

#include <QObject>
#include <QMap>
#include <QList>
#include "biotypes.h"
#include "uniauthservice.h"

class QDBusInterface;

#define SHARE_BIOMETRIC_CONFIG_PATH                                                                                    \
    "/var/lib/lightdm-data/%1/ukui-biometric.conf" // greeter、screensaver、polkit share conf

/*!
 * \brief The BioDevices class
 * this class saves the list of all device information
 * and list of available devices' info list for each uid
 */
class BioDevices : public QObject
{
    Q_OBJECT
public:
    explicit BioDevices(bool isIgnoreQrCode = false, QObject *parent = nullptr);

    int count();
    void setUId(int nUId);
    QMap<int, QList<DeviceInfo>> getAllDevices();
    QMap<int, QList<DeviceInfo>> getUserDevices(int uid);
    QList<DeviceInfo> getDevices(int type);
    DeviceInfoPtr getDefaultDevice(uid_t uid);
    DeviceInfoPtr getDefaultDevice(uid_t uid, int bioType);
    int GetLastDevice(const QString &userName);
    void SetLastDevice(const QString &userName, int drvid);
    DeviceInfoPtr findDevice(const QString &deviceName);
    DeviceInfoPtr findDevice(const int id);
    DeviceList findDeviceByType(int nBioType);
    DeviceInfoPtr getFirstDevice(int uid);
    DeviceList GetDevList();
    int getFeatureCount(int uid, int indexStart = 0, int indexEnd = -1);
    static QString bioTypeToString_tr(int type);
    void setIsShowHotPlug(bool isShow);
    int GetUserDevFeatureCount(int uid, int drvid);
    int GetUserDevCount(int uid);
    FeatureMap GetUserFeatures(int uid);
    bool getUseFirstDevice();
    int getFailedTimes();
    bool GetHiddenSwitchButton();
    bool GetQRCodeEnable();
    bool GetBioAuthEnable();
    QStringList getAllDefDevices();
    /**
     * @brief UpdateStatus  获取更新的设备状态
     * @param drvid 驱动id
     * @return  结果：<int result, int enable, int devNum,
     *                int devStatus, int opsStatus, notifyMessageId, ...>
     */
    StatusReslut UpdateStatus(int drvid);
    bool GetBioAuthEnable(uid_t uid);
    bool GetDirveIsIdle(int drvid);
    void recordAuthDrive(QString appName, int drvid, bool insert);

    QString getAuthMethod();

private:
    void connectToService();
    void getDevicesList();
    void updateListPriority(int uid = -1);

signals:
    void deviceCountChanged(int newNum);

private slots:
    void onUSBDeviceHotPlug(int deviceId, int action, int devNumNow);

private:
    QDBusInterface *serviceInterface;
    DeviceList deviceInfos; // the list of al device info
    bool isShowHotPlug;
    bool useFirstDevice;
    bool m_isIgnoreQrCode = false;
    UniAuthService *m_uniAuthService = nullptr;
    int m_nUId = -1;
    QList<int> m_listPriority;
};

#endif // BIODEVICES_H
