/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "bioauthenticate.h"
#include "../../src/dbusifs/freedesktophelper.h"
#include "../../src/dbusifs/biometrichelper.h"
#include "../../src/dbusifs/uniauthservice.h"
#include <pwd.h>
#include <unistd.h>
#include "../../src/dbusifs/giodbus.h"

BioAuthenticate::BioAuthenticate(QObject *parent)
    : QObject(parent), m_systemFdHelper(new FreedesktopHelper(false, this)), m_bioTimer(nullptr)
{
    m_bioauthState = BIOAUTH_IDLE;
    m_deviceInfo.id = -1;
    m_maxFailedTimes = UniAuthService::instance()->getMaxFailedTimes();
    m_listPriority.clear();
    m_listPriority.push_back(BioT_Face);
    m_listPriority.push_back(BioT_FingerPrint);
    m_listPriority.push_back(BioT_Iris);
    m_listPriority.push_back(BioT_VoicePrint);
    m_listPriority.push_back(BioT_FingerVein);
    m_listPriority.push_back(UniT_Remote);
    m_listPriority.push_back(UniT_General_Ukey);
    initBioService();
    initConncetions();
}

void BioAuthenticate::initConncetions()
{
    connect(m_systemFdHelper, &FreedesktopHelper::serviceStatusChanged, this, &BioAuthenticate::onServiceStatusChanged);
}

void BioAuthenticate::initBioService()
{
    if (!m_biometricHelper) {
        if (m_systemFdHelper->NameHasOwner(BIOMETRIC_DBUS_SERVICE)) {
            m_biometricHelper = new BiometricHelper(this);
            connect(m_biometricHelper, &BiometricHelper::USBDeviceHotPlug, this, &BioAuthenticate::onBioDeviceChanged);
            connect(m_biometricHelper, &BiometricHelper::StatusChanged, this, &BioAuthenticate::onStatusChanged);
            connect(m_biometricHelper, &BiometricHelper::FrameWritten, this, &BioAuthenticate::onFrameWritten);
        }
    }
}

DeviceMap BioAuthenticate::getAvailableDevices(int nUid)
{
    DeviceMap mapBioInfo;
    checkAvailableBioInfo(nUid);
    if (m_mapDevices.contains(nUid)) {
        mapBioInfo = m_mapDevices[nUid];
    }
    return mapBioInfo;
}

QList<int> BioAuthenticate::getDisabledDevices(int nUid)
{
    QList<int> listDisabled;
    if (m_mapDisableDev.contains(nUid)) {
        QMap<int, bool>::iterator devDisable = m_mapDisableDev[nUid].begin();
        for (; devDisable != m_mapDisableDev[nUid].end(); devDisable++) {
            if (devDisable.value()) {
                listDisabled.append(devDisable.key());
            }
        }
    }
    return listDisabled;
}

DeviceInfoPtr BioAuthenticate::findDeviceById(int nUid, int nDevId)
{
    DeviceInfoPtr devInfo;
    if (m_mapDevices.contains(nUid)) {
        DeviceMap::iterator itDevInfo = m_mapDevices[nUid].begin();
        for (; itDevInfo != m_mapDevices[nUid].end(); itDevInfo++) {
            if (itDevInfo.value()->id == nDevId) {
                devInfo = itDevInfo.value();
                break;
            }
        }
    }
    return devInfo;
}

DeviceInfoPtr BioAuthenticate::findDeviceByName(int nUid, QString strDevName)
{
    DeviceInfoPtr devInfo;
    if (m_mapDevices.contains(nUid)) {
        DeviceMap::iterator itDevInfo = m_mapDevices[nUid].begin();
        for (; itDevInfo != m_mapDevices[nUid].end(); itDevInfo++) {
            if (itDevInfo.value()->shortName == strDevName) {
                devInfo = itDevInfo.value();
                break;
            }
        }
    }
    return devInfo;
}

void BioAuthenticate::onServiceStatusChanged(const QString &strService, bool bActive)
{
    if (strService == BIOMETRIC_DBUS_SERVICE) {
        if (bActive) {
            initBioService();
        } else {
            clearBioData();
        }
        Q_EMIT bioServiceStatusChanged(bActive);
    }
}

bool BioAuthenticate::getBioAuthEnable(QString strUserName, int nType)
{
    bool isEnable = false;
    isEnable = UniAuthService::instance()->getBioAuthStatus(strUserName, ENABLETYPE_BIO);
    if (isEnable) {
        isEnable = UniAuthService::instance()->getBioAuthStatus(strUserName, nType);
    }
    return isEnable;
}

void BioAuthenticate::clearBioData()
{
    QMap<int, DeviceMap>::iterator itDeviceMap = m_mapDevices.begin();
    for (; itDeviceMap != m_mapDevices.end(); itDeviceMap++) {
        itDeviceMap.value().clear();
    }
    m_mapDevices.clear();
}

void BioAuthenticate::checkAvailableBioInfo(int nUid)
{
    if (!m_biometricHelper)
        return;
    if (m_mapDevices.contains(nUid)) {
        m_mapDevices[nUid].clear();
    }
    struct passwd *pwdInfo = getpwuid(nUid);
    if (pwdInfo) {
        int nAuthType = (pwdInfo->pw_name == getenv("USER")) ? ENABLETYPE_SAVER : ENABLETYPE_GREETER;
        bool isAuthEnable = getBioAuthEnable(pwdInfo->pw_name, nAuthType);
        bool isQRCodeEnable = UniAuthService::instance()->getQRCodeEnable();
        DeviceList deviceList = m_biometricHelper->GetDevList();
        QStringList listDefDevices = UniAuthService::instance()->getAllDefaultDevice(pwdInfo->pw_name);
        qDebug() << "BeginGetFeature------!";
        FeatureMap mapFeatures = m_biometricHelper->GetUserFeatures(nUid);
        qDebug() << nUid << ",count:" << mapFeatures.size();
        for (auto pDeviceInfo : deviceList) {
            if (!isAuthEnable && pDeviceInfo->deviceType <= BioT_VoicePrint)
                continue;
            if (!isQRCodeEnable && pDeviceInfo->deviceType == UniT_Remote)
                continue;
            int nFeatureCount = 0;
            if (mapFeatures.contains(pDeviceInfo->shortName) && listDefDevices.contains(pDeviceInfo->shortName)) {
                nFeatureCount = mapFeatures[pDeviceInfo->shortName].size();
                qDebug() << *pDeviceInfo << ",count:" << nFeatureCount;
                if (nFeatureCount > 0) {
                    if (m_mapDevices.contains(nUid)) {
                        if (!m_mapDevices[nUid].contains(pDeviceInfo->deviceType))
                            m_mapDevices[nUid][pDeviceInfo->deviceType] = pDeviceInfo;
                    } else {
                        DeviceMap mapBioInfo;
                        mapBioInfo[pDeviceInfo->deviceType] = pDeviceInfo;
                        m_mapDevices[nUid] = mapBioInfo;
                    }
                }
            }
        }
        FeatureMap::iterator itFeature = mapFeatures.begin();
        for (; itFeature != mapFeatures.end(); itFeature++) {
            for (auto feature : itFeature.value()) {
                if (feature->biotype == UniT_General_Ukey) {
                    DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
                    pDeviceInfo->id = -1;
                    pDeviceInfo->shortName = "GeneralUKey";
                    pDeviceInfo->deviceType = feature->biotype;
                    if (m_mapDevices.contains(nUid)) {
                        if (!m_mapDevices[nUid].contains(pDeviceInfo->deviceType))
                            m_mapDevices[nUid][pDeviceInfo->deviceType] = pDeviceInfo;
                    } else {
                        DeviceMap mapBioInfo;
                        mapBioInfo[pDeviceInfo->deviceType] = pDeviceInfo;
                        m_mapDevices[nUid] = mapBioInfo;
                    }
                    break;
                }
            }
        }
    }
}

void BioAuthenticate::onBioDeviceChanged(int drvid, int action, int devNum)
{
    Q_UNUSED(devNum);
    switch (action) {
        case ACTION_ATTACHED: {
            // 插入设备后，需要更新设备列表
            clearBioData();
            QMap<int, DeviceMap>::iterator itUserDev = m_mapDevices.begin();
            for (; itUserDev != m_mapDevices.end(); itUserDev++) {
                if (itUserDev.key() < 0)
                    continue;
                checkAvailableBioInfo(itUserDev.key());
            }
            break;
        }
        case ACTION_DETACHED: {
            if (m_deviceInfo.id != -1 && m_deviceInfo.id == drvid) {
                m_bioauthState = BIOAUTH_DEVICEREMOVED;
                Q_EMIT bioAuthStateChanged(BIOAUTH_DEVICEREMOVED);
            }
            clearBioData();
            QMap<int, DeviceMap>::iterator itUserDev = m_mapDevices.begin();
            for (; itUserDev != m_mapDevices.end(); itUserDev++) {
                if (itUserDev.key() < 0)
                    continue;
                checkAvailableBioInfo(itUserDev.key());
            }
            break;
        }
    }
    Q_EMIT bioDeviceChanged();
}

void BioAuthenticate::onStatusChanged(int drvid, int status)
{
    if (m_bioauthState != BIOAUTH_START || m_deviceInfo.id == -1) {
        return;
    }

    if (drvid != m_deviceInfo.id) {
        return;
    }

    //    // 显示来自服务的提示信息
    //    if(status == STATUS_NOTIFY) {
    //        QString notifyMsg = m_biometricHelper->GetNotifyMesg(drvid);
    //        Q_EMIT bioAuthShowMessage(notifyMsg);
    //    }
}

void BioAuthenticate::onFrameWritten(int drvid)
{
    if (m_deviceInfo.id == -1 || m_deviceInfo.id != drvid || m_bioauthState != BIOAUTH_START) {
        return;
    }
    if (m_fdFrame == -1) {
        m_fdFrame = get_server_gvariant_stdout(drvid);
    }

    if (m_fdFrame <= 0)
        return;

    lseek(m_fdFrame, 0, SEEK_SET);
    char base64_bufferData[1024 * 1024] = { 0 };
    int rc = read(m_fdFrame, base64_bufferData, 1024 * 1024);
    QByteArray byteData(base64_bufferData);
    QString strData = byteData.toBase64();
    Q_EMIT bioAuthFrameData(strData);
}

/**
 * @brief 进行生物识别认证
 * @param deviceInfo    使用的设备
 * @param uid           待认证的用户id
 */
void BioAuthenticate::startAuth(int uid, int nDevId)
{
    if (!m_biometricHelper) {
        qWarning() << "BiometricProxy doesn't exist.";
        return;
    }
    if (nDevId < 0) {
        qWarning() << "Biometric Auth device invalid.";
        return;
    }
    qDebug() << "deviceInfo:" << nDevId;
    DeviceInfoPtr ptrDeviceInfo = findDeviceById(uid, nDevId);
    if (!ptrDeviceInfo || ptrDeviceInfo->id < -1) {
        qDebug() << "Could not find device info!";
        return;
    }
    if (m_failedTimes.contains(uid) && m_failedTimes[uid].contains(ptrDeviceInfo->id)
        && m_failedTimes[uid][ptrDeviceInfo->id] >= m_maxFailedTimes) {
        qDebug() << "Failed MAX:" << uid << ptrDeviceInfo->id;
        Q_EMIT bioAuthCompleted(uid, false, -1, m_maxFailedTimes, m_failedTimes[uid][ptrDeviceInfo->id]);
        return;
    }
    if (m_bioauthState != BIOAUTH_IDLE) {
        qDebug() << "Identification is currently under way, stop it";
        stopAuth();
    } else {
        if (m_bioTimer && m_bioTimer->isActive())
            m_bioTimer->stop();
    }
    m_deviceInfo = *ptrDeviceInfo;
    m_nUid = uid;
    m_isStopped = false;
    m_fTimeoutTimes = 0;
    m_bioauthState = BIOAUTH_START;
    _startAuth();
    Q_EMIT bioAuthStateChanged(m_bioauthState);
}

void BioAuthenticate::_startAuth()
{
    if (m_nUid < 0 || m_deviceInfo.id < 0) {
        return;
    }
    if (m_deviceInfo.deviceType == UniT_General_Ukey) {
        qDebug() << QString("Identify:[drvid: %1, uid: %2]").arg(m_deviceInfo.id).arg(m_nUid);
        m_fdFrame = -1;
        m_biometricHelper->StopOps(m_deviceInfo.id);
        QDBusPendingCall call = m_biometricHelper->UkeyIdentify(m_deviceInfo.id, 2, m_nUid);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuthenticate::onIdentifyComplete);
    } else if (
        (m_deviceInfo.deviceType >= BioT_FingerPrint && m_deviceInfo.deviceType <= BioT_VoicePrint)
        || m_deviceInfo.deviceType == UniT_Remote) {
        qDebug() << QString("Identify:[drvid: %1, uid: %2]").arg(m_deviceInfo.id).arg(m_nUid);
        m_fdFrame = -1;
        m_biometricHelper->StopOps(m_deviceInfo.id);
        QDBusPendingCall call = m_biometricHelper->Identify(m_deviceInfo.id, m_nUid);
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &BioAuthenticate::onIdentifyComplete);
    }
}

void BioAuthenticate::SetExtraInfo(QString extra_info, QString info_type)
{
    if (!m_biometricHelper) {
        qWarning() << "m_biometricHelper doesn't exist.";
        return;
    }
    m_biometricHelper->SetExtraInfo(info_type, extra_info);
}

void BioAuthenticate::onIdentifyComplete(QDBusPendingCallWatcher *watcher)
{
    if (!m_biometricHelper || m_isStopped)
        return;

    QDBusPendingReply<int, int> reply = *watcher;
    if (reply.isError()) {
        qWarning() << "Identify error: " << reply.error().message();
        onBioAuthComplete(false, -1);
        return;
    }
    int result = reply.argumentAt(0).toInt();
    int authUid = reply.argumentAt(1).toInt();
    qDebug() << result << authUid << m_nUid;

    // 特征识别成功，而且用户id匹配
    if (result == DBUS_RESULT_SUCCESS && authUid == m_nUid) {
        qDebug() << "Identify success";
        onBioAuthComplete(true, 0);
    } else if (result == DBUS_RESULT_NOTMATCH) { // 特征识别不匹配
        qDebug() << "Identify failed";
        onBioAuthComplete(false, 2);
    } else if (result == DBUS_RESULT_ERROR) { // 识别发生错误
        StatusReslut ret = m_biometricHelper->UpdateStatus(m_deviceInfo.id);
        qDebug() << "StatusReslut:" << ret.result << "," << ret.enable << "," << ret.devNum << "," << ret.devStatus
                 << "," << ret.opsStatus << "," << ret.notifyMessageId;
        // 识别操作超时 304/404 认证超时；8 网络错误；
        if (ret.result == 0) {
            if (ret.opsStatus == OPS_IDENTIFY_TIMEOUT || ret.opsStatus == OPS_VERIFY_TIMEOUT
                || ret.opsStatus == 8) { // 304认证超时， 8网络异常
                m_bioauthState = BIOAUTH_TIMEOUT;
                Q_EMIT bioAuthStateChanged(m_bioauthState);
                onBioAuthComplete(false, 1);
            } else if (ret.opsStatus == OPS_IDENTIFY_STOP_BY_USER || ret.opsStatus == OPS_VERIFY_STOP_BY_USER) {
                onBioAuthComplete(false, -2); // 主动停止，直接重试
            } else if (
                ret.opsStatus == OPS_OPEN_FAIL
                || ret.opsStatus == OPS_OPEN_ERROR) { // 无法打开设备（设备是坏的/被占用），直接停止
                onBioAuthComplete(false, 5);
            } else if (ret.opsStatus >= OPS_GET_FLIST_SUCCESS && ret.opsStatus <= OPS_GET_FLIST_MAX) {
                onBioAuthComplete(false, -3); // 主动停止，直接重试
            } else {
                onBioAuthComplete(false, 2);
            }
        } else {
            onBioAuthComplete(false, 2);
        }
    } else {
        onBioAuthComplete(false, 2);
    }
}

/**
 * @brief 终止生物识别认证
 */
void BioAuthenticate::stopAuth()
{
    if (!m_biometricHelper) {
        return;
    }
    m_isStopped = true;
    if (m_bioTimer && m_bioTimer->isActive())
        m_bioTimer->stop();
    if (m_bioauthState == BIOAUTH_IDLE || m_deviceInfo.id < 0) {
        return;
    }
    m_biometricHelper->StopOps(m_deviceInfo.id);
    m_bioauthState = BIOAUTH_IDLE;
    Q_EMIT bioAuthStateChanged(m_bioauthState);
}

void BioAuthenticate::onBioAuthComplete(bool isSuccess, int nError)
{
    m_bioauthState = BIOAUTH_COMPLETE;
    Q_EMIT bioAuthStateChanged(m_bioauthState);
    if (!isSuccess) {
        if (nError == 5 && m_deviceInfo.id > -1) {
            Q_EMIT bioAuthCompleted(m_nUid, false, nError, m_maxFailedTimes, 0);
            return;
        } else if (nError >= 2 && nError != 5)
            if (m_deviceInfo.id > -1) {
                if (m_failedTimes.contains(m_nUid)) {
                    if (m_failedTimes[m_nUid].contains(m_deviceInfo.id)) {
                        m_failedTimes[m_nUid][m_deviceInfo.id] = m_failedTimes[m_nUid][m_deviceInfo.id] + 1;
                    } else {
                        m_failedTimes[m_nUid][m_deviceInfo.id] = 1;
                    }
                } else {
                    m_failedTimes[m_nUid][m_deviceInfo.id] = 1;
                }
                qDebug() << "Failed count:" << m_failedTimes[m_nUid][m_deviceInfo.id] << ",Max:" << m_maxFailedTimes;
                if (m_failedTimes[m_nUid][m_deviceInfo.id] >= m_maxFailedTimes) {
                    m_mapDisableDev[m_nUid][m_deviceInfo.id] = true;
                    Q_EMIT bioAuthCompleted(
                        m_nUid, false, nError, m_maxFailedTimes, m_failedTimes[m_nUid][m_deviceInfo.id]);
                    return;
                }
                Q_EMIT bioAuthCompleted(
                    m_nUid, false, nError, m_maxFailedTimes, m_failedTimes[m_nUid][m_deviceInfo.id]);
            }
        if (nError <= 0) {
            qDebug() << "Biometric dbus error:" << nError;
        }
        if (m_deviceInfo.deviceType == UniT_Remote && nError == 1) {
            startBioAuth(10000);
        } else if (m_deviceInfo.deviceType == LOGINOPT_TYPE_FACE && nError == 1) {
            m_fTimeoutTimes += 1;
            if (m_fTimeoutTimes == UniAuthService::instance()->getFTimeoutTimes()) {
                m_fTimeoutTimes = 0;
                nError = 6;
            } else {
                startBioAuth();
            }
        } else {
            startBioAuth();
        }
        Q_EMIT bioAuthCompleted(m_nUid, false, nError, m_maxFailedTimes, m_failedTimes[m_nUid][m_deviceInfo.id]);
    } else {
        Q_EMIT bioAuthCompleted(m_nUid, true, 0, m_maxFailedTimes, 0);
    }
    m_bioauthState = BIOAUTH_IDLE;
    Q_EMIT bioAuthStateChanged(m_bioauthState);
}

void BioAuthenticate::onPamAuthComplete()
{
    QMap<int, QMap<int, int>>::iterator itFaildTimes = m_failedTimes.begin();
    for (; itFaildTimes != m_failedTimes.end(); itFaildTimes++) {
        itFaildTimes.value().clear();
    }
    QMap<int, QMap<int, bool>>::iterator itDisableDev = m_mapDisableDev.begin();
    for (; itDisableDev != m_mapDisableDev.end(); itDisableDev++) {
        itDisableDev.value().clear();
    }
}

void BioAuthenticate::onBioAuthTimer()
{
    m_bioTimer->stop();
    if (m_deviceInfo.id < 0) {
        return;
    }

    if (m_deviceInfo.id > -1 && m_deviceInfo.deviceType == LOGINOPT_TYPE_GENERAL_UKEY) {
        // ukey时不调用ukey认证
    } else {
        _startAuth();
    }
}

void BioAuthenticate::startBioAuth(unsigned uTimeout)
{
    stopAuth();

    if (m_deviceInfo.id > -1 && m_deviceInfo.deviceType == UniT_General_Ukey) {
        // ukey时不调用ukey认证
        m_bioauthState = BIOAUTH_START;
        Q_EMIT bioAuthStateChanged(m_bioauthState);
        return;
    }

    if (!m_bioTimer) {
        m_bioTimer = new QTimer(this);
        connect(m_bioTimer, SIGNAL(timeout()), this, SLOT(onBioAuthTimer()));
    }
    m_bioTimer->start(uTimeout);
}

QString BioAuthenticate::getDefaultDevice(int nUid, QString strUserName, int bioType)
{
    QString defaultDeviceName = "";
    if (UniAuthService::instance()->isActivatable()) {
        QString strDeviceName = UniAuthService::instance()->getDefaultDevice(strUserName, bioType);
        if (!strDeviceName.isEmpty()) {
            DeviceInfoPtr pDeviceInfo = findDeviceByName(nUid, strDeviceName);
            if (pDeviceInfo) {
                defaultDeviceName = strDeviceName;
            }
        }
    }
    return QString(defaultDeviceName);
}

QString BioAuthenticate::getDefaultDevice(int nUid, QString strUserName)
{
    QString defaultDeviceName = "";
    if (UniAuthService::instance()->isActivatable()) {
        for (auto bioType : m_listPriority) {
            QString strDeviceName = UniAuthService::instance()->getDefaultDevice(strUserName, bioType);
            if (!strDeviceName.isEmpty()) {
                DeviceInfoPtr pDeviceInfo = findDeviceByName(nUid, strDeviceName);
                if (pDeviceInfo) {
                    defaultDeviceName = strDeviceName;
                    break;
                }
            }
        }
    }
    return QString(defaultDeviceName);
}
