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
#ifndef BIOAUTHENTICATE_H
#define BIOAUTHENTICATE_H

#include <QObject>
#include <QList>
#include <QMap>
#include "../../src/common/biodefines.h"

class BiometricHelper;
class FreedesktopHelper;
class UniAuthService;
class BioAuthenticate : public QObject
{
    Q_OBJECT
public:
    explicit BioAuthenticate(QObject *parent = nullptr);

    DeviceMap getAvailableDevices(int nUid);
    QList<int> getDisabledDevices(int nUid);
    inline BIOAUTH_STATE getBioAuthState()
    {
        return m_bioauthState;
    }
    inline DeviceInfo getCurBioInfo()
    {
        return m_deviceInfo;
    }
    DeviceInfoPtr findDeviceById(int nUid, int nDevId);
    DeviceInfoPtr findDeviceByName(int nUid, QString strDevName);
    void startAuth(int uid, int nDevId);
    void stopAuth();
    QString getDefaultDevice(int nUid, QString strUserName);
    QString getDefaultDevice(int nUid, QString strUserName, int bioType);

public Q_SLOTS:
    void onServiceStatusChanged(const QString &strService, bool bActive);
    void onBioDeviceChanged(int drvid, int action, int devNum);
    void onStatusChanged(int drvid, int status);
    void onFrameWritten(int drvid);
    void onIdentifyComplete(QDBusPendingCallWatcher *watcher);
    void onBioAuthComplete(bool isSuccess, int nError);
    void onPamAuthComplete();
    void onBioAuthTimer();

Q_SIGNALS:
    void bioServiceStatusChanged(bool bValid);
    void bioDeviceChanged();
    void bioAuthShowMessage(QString strMsg);
    void bioAuthStateChanged(int nState);
    void bioAuthFrameData(QString strData);
    void bioAuthCompleted(int nUid, bool isSuccess, int nError, int nMaxFailedTime, int nFailedTime);

private:
    void initBioService();
    void initConncetions();

    bool getBioAuthEnable(QString strUserName, int nType);
    void checkAvailableBioInfo(int nUid);
    void clearBioData();
    void SetExtraInfo(QString extra_info, QString info_type);
    void startBioAuth(unsigned uTimeout = 1000);
    void _startAuth();

private:
    BiometricHelper *m_biometricHelper = nullptr;
    FreedesktopHelper *m_systemFdHelper = nullptr;
    QMap<int, DeviceMap> m_mapDevices; // 用户可用的设备
    DeviceInfo m_deviceInfo;
    BIOAUTH_STATE m_bioauthState;
    int m_fdFrame = -1;
    int m_nUid = -1;
    bool m_isStopped = false; // 是否被强制终止
    int m_maxFailedTimes;
    QMap<int, QMap<int, bool>> m_mapDisableDev;
    QMap<int, QMap<int, int>> m_failedTimes;
    QTimer *m_bioTimer;
    int m_fTimeoutTimes = 0;
    QList<int> m_listPriority;
};

#endif // BIOAUTHENTICATE_H
