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
#include "biodevices.h"
#include <QDBusInterface>
#include <QSettings>
#include <QFile>

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "generic.h"

BioDevices::BioDevices(bool isIgnoreQrCode, QObject *parent)
    : QObject(parent)
    , m_uniAuthService(new UniAuthService(this))
    , isShowHotPlug(false)
    , useFirstDevice(false)
    , m_isIgnoreQrCode(isIgnoreQrCode)
{
    connectToService();

    updateListPriority();
}

void BioDevices::setUId(int nUId)
{
    m_nUId = nUId;
    updateListPriority();
    getDevicesList();
}

void BioDevices::updateListPriority(int uid)
{
    m_listPriority.clear();
    QString authMethod = getAuthMethod();
    if (getProductNameN90F3() && (authMethod != "otp" || authMethod != "ukey")) {
        struct passwd *pwdInfo;
        if (uid != -1) {
            pwdInfo = getpwuid(m_nUId);
        } else {
            pwdInfo = getpwuid(uid);
        }
        if (pwdInfo) {
            if (m_uniAuthService && m_uniAuthService->isActivatable()) {
                int defaultAuthType = m_uniAuthService->getDefaultAuthType(pwdInfo->pw_name);
                m_listPriority.push_back(defaultAuthType);
            }
        }
    }
    if (!m_listPriority.contains(BIOTYPE_FACE)) {
        m_listPriority.push_back(BIOTYPE_FACE);
    }
    if (!m_listPriority.contains(BIOTYPE_FINGERPRINT)) {
        m_listPriority.push_back(BIOTYPE_FINGERPRINT);
    }
    if (!m_listPriority.contains(BIOTYPE_IRIS)) {
        m_listPriority.push_back(BIOTYPE_IRIS);
    }
    if (!m_listPriority.contains(BIOTYPE_VOICEPRINT)) {
        m_listPriority.push_back(BIOTYPE_VOICEPRINT);
    }
    if (!m_listPriority.contains(BIOTYPE_FINGERVEIN)) {
        m_listPriority.push_back(BIOTYPE_FINGERVEIN);
    }
    if (!m_listPriority.contains(-1)) {
        m_listPriority.push_back(-1);  // 密码类型
    }
    if (!m_listPriority.contains(REMOTE_QRCODE_TYPE)) {
        m_listPriority.push_back(REMOTE_QRCODE_TYPE);
    }
    if (!m_listPriority.contains(LOGINOPT_TYPE_GENERAL_UKEY)) {
        m_listPriority.push_back(LOGINOPT_TYPE_GENERAL_UKEY);
    }
}

void BioDevices::connectToService()
{
    qRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<DeviceInfo>();

    serviceInterface
        = new QDBusInterface(BIO_DBUS_SERVICE, BIO_DBUS_PATH, BIO_DBUS_INTERFACE, QDBusConnection::systemBus());

    connect(serviceInterface, SIGNAL(USBDeviceHotPlug(int, int, int)), this, SLOT(onUSBDeviceHotPlug(int, int, int)));
    serviceInterface->setTimeout(2147483647);
}

void BioDevices::onUSBDeviceHotPlug(int deviceId, int action, int devNumNow)
{
    qDebug() << deviceId << action << devNumNow;
    QString text = "";

    if (action == -1) {
        DeviceInfoPtr device = findDevice(deviceId);
        if (device)
            text = tr("Unplugging of %1 device detected").arg(bioTypeToString_tr(device->biotype));
    }

    getDevicesList();

    emit deviceCountChanged(deviceInfos.size());

    if (action == 1) {
        DeviceInfoPtr device = findDevice(deviceId);
        if (device)
            text = tr("%1 device insertion detected").arg(bioTypeToString_tr(device->biotype));
    }

    if (isShowHotPlug && text != "") {
        QDBusInterface iface(
            "org.freedesktop.Notifications",
            "/org/freedesktop/Notifications",
            "org.freedesktop.Notifications",
            QDBusConnection::sessionBus());
        QList<QVariant> args;
        args << (tr("Biometric Manager")) << ((unsigned int)0) << "biometric-manager" << tr("biometric") << text
             << QStringList() << QVariantMap() << (int)-1;
        iface.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    }
}

bool BioDevices::GetBioAuthEnable(uid_t uid)
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        struct passwd *pwd = getpwuid(uid);
        if (pwd) {
            return m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_BIO);
        } else {
            return false;
        }
    } else {
        QString configPath = "/etc/biometric-auth/ukui-biometric.conf";
        QSettings settings(configPath, QSettings::IniFormat);
        qDebug() << "configure path: " << settings.fileName();

        if (settings.allKeys().contains("EnableAuth")) {
            return settings.value("EnableAuth").toBool();
        } else {
            QSettings sysSettings(GET_STR(CONFIG_FILE), QSettings::IniFormat);
            if (sysSettings.contains("EnableAuth"))
                return sysSettings.value("EnableAuth").toBool();
            else
                return true;
        }
    }
}

/**
 * 获取设备列表
 */
void BioDevices::getDevicesList()
{
    /* 返回值为 i -- int 和 av -- array of variant */
    QDBusMessage msg = serviceInterface->call("GetDevList");
    if (msg.type() == QDBusMessage::ErrorMessage) {
        LOG() << msg.errorMessage();
        return;
    }
    /* 设备数量 */
    int deviceNum = msg.arguments().at(0).toInt();
    bool bEnableBioAuth = GetBioAuthEnable();

    /* 读取设备详细信息，并存储到列表中 */
    QDBusArgument argument = msg.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> infos;
    argument >> infos;

    deviceInfos.clear();

    for (int i = 0; i < deviceNum; i++) {
        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
        infos.at(i).value<QDBusArgument>() >> *pDeviceInfo;

        if (pDeviceInfo->device_available > 0) { // 设备可用
            if (pDeviceInfo->biotype == REMOTE_QRCODE_TYPE && m_isIgnoreQrCode)
                continue;
            if (!bEnableBioAuth && pDeviceInfo->biotype <= BIOTYPE_VOICEPRINT)
                continue;
            deviceInfos.push_back(pDeviceInfo);
        }
    }
}

int BioDevices::count()
{
    return deviceInfos.size();
}

int BioDevices::GetUserDevCount(int uid)
{
    int count = 0;
    QDBusMessage msg = serviceInterface->call("GetDevList");
    if (msg.type() == QDBusMessage::ErrorMessage) {
        LOG() << msg.errorMessage();
        return 0;
    }

    int deviceNum = msg.arguments().at(0).toInt();

    QDBusArgument argument = msg.arguments().at(1).value<QDBusArgument>();
    QList<QVariant> infos;
    argument >> infos;

    for (int i = 0; i < deviceNum; i++) {
        DeviceInfoPtr pDeviceInfo = std::make_shared<DeviceInfo>();
        infos.at(i).value<QDBusArgument>() >> *pDeviceInfo;
        if (pDeviceInfo->device_available > 0 && GetUserDevFeatureCount(uid, pDeviceInfo->device_id) > 0) // 设备可用
            count++;
    }
    return count;
}

FeatureMap BioDevices::GetUserFeatures(int uid)
{
    FeatureMap featureMap;
    QList<QDBusVariant> qlist;
    int listsize;
    QDBusMessage result = serviceInterface->call(QStringLiteral("GetAllFeatureList"), uid, 0, -1);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetDevList error:" << result.errorMessage();
        return featureMap;
    }

    QList<QVariant> variantList = result.arguments();
    listsize = variantList[0].value<int>();
    variantList[1].value<QDBusArgument>() >> qlist;
    for (int i = 0; i < listsize; i++) {
        FeatureInfoPtr pFeatureInfo = std::make_shared<FeatureInfo>();
        qlist[i].variant().value<QDBusArgument>() >> *pFeatureInfo;
        featureMap[pFeatureInfo->device_shortname].append(pFeatureInfo);
    }

    return featureMap;
}

int BioDevices::GetUserDevFeatureCount(int uid, int drvid)
{
    QDBusMessage FeatureResult = serviceInterface->call(QStringLiteral("GetFeatureList"), drvid, uid, 0, -1);
    if (FeatureResult.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "GetFeatureList error:" << FeatureResult.errorMessage();
        return 0;
    }
    return FeatureResult.arguments().takeFirst().toInt();
}

int BioDevices::getFeatureCount(int uid, int indexStart, int indexEnd)
{
    int res = 0;
    for (int i = 0; i < deviceInfos.count(); i++) {
        DeviceInfoPtr deviceInfo = deviceInfos.at(i);
        QDBusMessage featurecount
            = serviceInterface->call("GetFeatureList", deviceInfo->device_id, uid, indexStart, indexEnd);
        if (featurecount.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "GetFeatureList error:" << featurecount.errorMessage();
        } else {
            res += featurecount.arguments().takeFirst().toInt();
        }
    }
    return res;
}

QMap<int, QList<DeviceInfo>> BioDevices::getAllDevices()
{
    QMap<int, QList<DeviceInfo>> devices;

    for (auto deviceInfo : deviceInfos) {
        if (deviceInfo->biotype == REMOTE_QRCODE_TYPE) // 终端不使用扫码
            continue;

        if (!GetDirveIsIdle(deviceInfo->device_id))
            continue;

        devices[deviceInfo->biotype].push_back(*deviceInfo);
    }

    return devices;
}

QMap<int, QList<DeviceInfo>> BioDevices::getUserDevices(int uid)
{
    QMap<int, QList<DeviceInfo>> devices;
    bool isQRCodeEnable = GetQRCodeEnable();
    for (auto deviceInfo : deviceInfos) {
        if (GetUserDevFeatureCount(uid, deviceInfo->device_id) > 0) {
            if (!isQRCodeEnable && deviceInfo->biotype == REMOTE_QRCODE_TYPE) // 关闭了二维码功能
                continue;
            devices[deviceInfo->biotype].push_back(*deviceInfo);
        }
    }

    return devices;
}

QList<DeviceInfo> BioDevices::getDevices(int type)
{
    QList<DeviceInfo> devices;

    for (auto deviceInfo : deviceInfos) {
        if (deviceInfo->biotype == type)
            devices.push_back(*deviceInfo);
    }

    return devices;
}

void BioDevices::setIsShowHotPlug(bool isShow)
{
    isShowHotPlug = isShow;
}

bool BioDevices::getUseFirstDevice()
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        return m_uniAuthService->getUseFirstDevice();
    } else {
        QSettings settings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        return settings.value("UseFirstDevice").toBool();
    }
}

bool BioDevices::GetHiddenSwitchButton()
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        return m_uniAuthService->getHiddenSwitchButton();
    } else {
        QSettings sysSettings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        if (sysSettings.contains("HiddenSwitchButton"))
            return sysSettings.value("HiddenSwitchButton").toBool();
        else
            return false;
    }
}

int BioDevices::getFailedTimes()
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        return m_uniAuthService->getMaxFailedTimes();
    } else {
        QSettings sysSettings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        if (sysSettings.contains("MaxFailedTimes"))
            return sysSettings.value("MaxFailedTimes").toInt();
        else
            return 3;
    }
}

bool BioDevices::GetQRCodeEnable()
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        return m_uniAuthService->getQRCodeEnable();
    } else {
        bool isEnable = false;
        QSettings sysSettings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        sysSettings.beginGroup("Functions");
        if (sysSettings.allKeys().contains("EnableQRCode")) {
            isEnable = sysSettings.value("EnableQRCode").toBool();
        }
        sysSettings.endGroup();
        return isEnable;
    }
}

/**
 * ukui-greeter         1
 * ukui-screensaver     1<<1
 * ukui-polkit          1<<2
 * sudo                 1<<3
 * su                   1<<4
 * login                1<<5
 **/
bool BioDevices::GetBioAuthEnable()
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        bool isEnable = false;
        struct passwd *pwd = getpwuid(m_nUId);
        if (pwd) {
            isEnable = m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_BIO);
        }
        if (isEnable && !isShowHotPlug) {
            if (qAppName() == "polkit-ukui-authentication-agent-1") // 获取polkit开关是否打开
                isEnable = m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_POLKIT);
            if (qAppName() == "bioauth") { // 获取父进程名称为sudo，su，或者login
                int ppid = getppid();      // 获取父进程pid
                QString filename = "/proc/" + QString::number(ppid) + "/cmdline";
                QFile file(filename);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qDebug() << "Can't open the file!" << endl;
                }
                QByteArray text = file.readAll();
                file.close();
                if (text.contains("sudo")) {
                    isEnable = m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_SUDO);
                } else if (text.contains("su")) {
                    isEnable = m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_SU);
                } else if (text.contains("login")) {
                    isEnable = m_uniAuthService->getBioAuthStatus(pwd->pw_name, ENABLETYPE_LOGIN);
                }
            }
        }

        return isEnable;
    } else {
        bool isEnable = false;
        int isEnableApp = 0;
        QSettings sysSettings("/etc/biometric-auth/ukui-biometric.conf", QSettings::IniFormat);
        if (sysSettings.allKeys().contains("EnableAuth")) {
            isEnable = sysSettings.value("EnableAuth").toBool();
        }

        if (isEnable && sysSettings.allKeys().contains("EnableAuthApp")) {
            isEnableApp = sysSettings.value("EnableAuthApp").toInt();
            if (qAppName() == "polkit-ukui-authentication-agent-1") // 获取polkit开关是否打开
                isEnable = isEnableApp & (1 << 2);
            if (qAppName() == "bioauth") { // 获取父进程名称为sudo，su，或者login
                int ppid = getppid();      // 获取父进程pid
                QString filename = "/proc/" + QString::number(ppid) + "/cmdline";
                QFile file(filename);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    qDebug() << "Can't open the file!" << endl;
                }
                QByteArray text = file.readAll();
                file.close();
                if (text.contains("sudo")) {
                    isEnable = isEnableApp & (1 << 3);
                } else if (text.contains("su")) {
                    isEnable = isEnableApp & (1 << 4);
                } else if (text.contains("login")) {
                    isEnable = isEnableApp & (1 << 5);
                }
            }
        }

        return isEnable;
    }
}

DeviceInfoPtr BioDevices::getDefaultDevice(uid_t uid, int bioType)
{
    if (deviceInfos.size() <= 0)
        return nullptr;

    FeatureMap mapFeatures = GetUserFeatures(uid);
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        QString defaultDeviceName = "";
        struct passwd *pwdInfo = getpwuid(uid);
        DeviceInfoPtr ptrDevInfo = nullptr;
        if (pwdInfo) {
            QString strDeviceName = m_uniAuthService->getDefaultDevice(pwdInfo->pw_name, bioType);
            qDebug() << strDeviceName;
            if (!strDeviceName.isEmpty()) {
                ptrDevInfo = findDevice(strDeviceName);
                if (ptrDevInfo) {
                    int isIdle = GetDirveIsIdle(ptrDevInfo->device_id);
                    if (isIdle && mapFeatures.contains(ptrDevInfo->device_shortname)) {
                        defaultDeviceName = strDeviceName;
                    }
                }
            }
        }
        if (defaultDeviceName.isEmpty()) {
            return nullptr;
        }
        return ptrDevInfo;
    } else {
        QString defaultDeviceName;

        struct passwd *pwd = getpwuid(uid);
        QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
        QSettings userConfig(userConfigFile, QSettings::IniFormat);
        qDebug() << userConfig.fileName();
        defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        qDebug() << defaultDeviceName;

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QSettings sysConfig(GET_STR(CONFIG_FILE), QSettings::IniFormat);
            defaultDeviceName = sysConfig.value(DEFAULT_DEVICE).toString();
        }

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
            QSettings userConfig(userConfigFile, QSettings::IniFormat);
            defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        }
        qDebug() << "default device: " << defaultDeviceName;
        // 终端不默认使用第一个设备
        if (defaultDeviceName.isEmpty()) {
            return nullptr;
        }
        bool defValid = false;
        DeviceInfoPtr ptrDevInfo = findDevice(defaultDeviceName);
        if (ptrDevInfo) {
            int isIdle = GetDirveIsIdle(ptrDevInfo->device_id);
            if (isIdle && mapFeatures.contains(ptrDevInfo->device_shortname)) {
                defValid = true;
            }
        }

        if (!defValid) {
            ptrDevInfo = nullptr;
        }

        return ptrDevInfo;
    }
}

void BioDevices::recordAuthDrive(QString appName, int drvid, bool insert)
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        m_uniAuthService->recordAuthDrive(appName, drvid, insert);
    }
}

bool BioDevices::GetDirveIsIdle(int drvid)
{
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        QStringList appList = m_uniAuthService->getAuthingApp();
        for (QString appName : appList) {
            int id = m_uniAuthService->getAuthingAppDrivd(appName);
            if (id == drvid) {
                return false;
            }
        }
    }

    return true;
}

DeviceInfoPtr BioDevices::getDefaultDevice(uid_t uid)
{
    updateListPriority(uid);
    if (deviceInfos.size() <= 0)
        return nullptr;

    FeatureMap mapFeatures = GetUserFeatures(uid);
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        QString defaultDeviceName = "";
        struct passwd *pwdInfo = getpwuid(uid);
        DeviceInfoPtr ptrDevInfo = nullptr;
        if (pwdInfo) {
            for (auto bioType : m_listPriority) {
                QString authMethod = getAuthMethod();
                if (getProductNameN90F3() && (authMethod != "otp" || authMethod != "ukey")) {
                    int defaultAuthType = m_uniAuthService->getDefaultAuthType(pwdInfo->pw_name);
                    if (defaultAuthType == -1) {
                        return nullptr;
                    }
                }
                if (-1 == bioType) {
                    return nullptr;
                } else if (bioType == REMOTE_QRCODE_TYPE) {
                    DeviceList listDevice = findDeviceByType(bioType);
                    for (auto device : listDevice) {
                        int isIdle = GetDirveIsIdle(device->device_id);
                        if (isIdle && mapFeatures.contains(device->device_shortname)) {
                            defaultDeviceName = device->device_shortname;
                            ptrDevInfo = device;
                            break;
                        }
                    }
                    qDebug() << "defaultDevice:" << defaultDeviceName;
                    if (!defaultDeviceName.isEmpty()) {
                        break;
                    }
                } else {
                    QString strDeviceName = m_uniAuthService->getDefaultDevice(pwdInfo->pw_name, bioType);
                    qDebug() << strDeviceName;
                    if (!strDeviceName.isEmpty()) {
                        ptrDevInfo = findDevice(strDeviceName);
                        if (ptrDevInfo) {
                            int isIdle = GetDirveIsIdle(ptrDevInfo->device_id);
                            if (isIdle && mapFeatures.contains(ptrDevInfo->device_shortname)) {
                                defaultDeviceName = strDeviceName;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (defaultDeviceName.isEmpty()) {
            return nullptr;
        }
        return ptrDevInfo;
    } else {
        QString defaultDeviceName;

        struct passwd *pwd = getpwuid(uid);
        QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
        QSettings userConfig(userConfigFile, QSettings::IniFormat);
        qDebug() << userConfig.fileName();
        defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        qDebug() << defaultDeviceName;

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QSettings sysConfig(GET_STR(CONFIG_FILE), QSettings::IniFormat);
            defaultDeviceName = sysConfig.value(DEFAULT_DEVICE).toString();
        }

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
            QSettings userConfig(userConfigFile, QSettings::IniFormat);
            defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        }
        qDebug() << "default device: " << defaultDeviceName;
        // 终端不默认使用第一个设备
        if (defaultDeviceName.isEmpty()) {
            return nullptr;
        }
        bool defValid = false;
        DeviceInfoPtr ptrDevInfo = findDevice(defaultDeviceName);
        if (ptrDevInfo) {
            if (mapFeatures.contains(ptrDevInfo->device_shortname)) {
                defValid = true;
            }
        }

        if (!defValid) {
            ptrDevInfo = nullptr;
        }

        return ptrDevInfo;
    }
}

QStringList BioDevices::getAllDefDevices()
{
    QStringList listDefDevices;
    if (m_uniAuthService && m_uniAuthService->isActivatable()) {
        struct passwd *pwdInfo = getpwuid(m_nUId);
        if (pwdInfo) {
            listDefDevices = m_uniAuthService->getAllDefaultDevice(pwdInfo->pw_name);
        }
    } else {
        QString defaultDeviceName;

        struct passwd *pwd = getpwuid(m_nUId);
        QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
        QSettings userConfig(userConfigFile, QSettings::IniFormat);
        qDebug() << userConfig.fileName();
        defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        qDebug() << defaultDeviceName;

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QSettings sysConfig(GET_STR(CONFIG_FILE), QSettings::IniFormat);
            defaultDeviceName = sysConfig.value(DEFAULT_DEVICE).toString();
        }

        if (defaultDeviceName.isEmpty() || !findDevice(defaultDeviceName)) {
            QString userConfigFile = QString(pwd->pw_dir) + "/.biometric_auth/ukui_biometric.conf";
            QSettings userConfig(userConfigFile, QSettings::IniFormat);
            defaultDeviceName = userConfig.value(DEFAULT_DEVICE).toString();
        }
        qDebug() << "default device: " << defaultDeviceName;
        if (!defaultDeviceName.isEmpty()) {
            listDefDevices.push_back(defaultDeviceName);
        }
    }
    return listDefDevices;
}

int BioDevices::GetLastDevice(const QString &userName)
{
    int nLastDevId = -1;
    QSettings sysSettings(QString(SHARE_BIOMETRIC_CONFIG_PATH).arg(userName), QSettings::IniFormat);
    sysSettings.beginGroup("Common");
    if (sysSettings.allKeys().contains("LastDeviceId")) {
        nLastDevId = sysSettings.value("LastDeviceId").toInt();
    }
    sysSettings.endGroup();
    return nLastDevId;
}

void BioDevices::SetLastDevice(const QString &userName, int drvid)
{
    if (drvid < 0) {
        return;
    }
    QString desConfPath = QString(SHARE_BIOMETRIC_CONFIG_PATH).arg(userName);
    QFile fileConf(desConfPath);
    if (fileConf.exists()) {
        QSettings sysSettings(desConfPath, QSettings::IniFormat);
        sysSettings.beginGroup("Common");
        sysSettings.setValue("LastDeviceId", drvid);
        sysSettings.endGroup();
    } else {
        QSettings sysSettings(desConfPath, QSettings::IniFormat);
        sysSettings.beginGroup("Common");
        sysSettings.setValue("LastDeviceId", drvid);
        sysSettings.endGroup();
        sysSettings.sync();
        QFile file(desConfPath);
        file.setPermissions(QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | QFile::ReadOther);
    }
}

DeviceInfoPtr BioDevices::findDevice(const int id)
{
    for (auto deviceInfo : deviceInfos) {
        if (deviceInfo->device_id == id)
            return deviceInfo;
    }
    // qDebug() << deviceName << "doesn't exists";
    return nullptr;
}

DeviceInfoPtr BioDevices::findDevice(const QString &deviceName)
{
    for (auto deviceInfo : deviceInfos) {
        if (deviceInfo->device_shortname == deviceName)
            return deviceInfo;
    }
    qDebug() << deviceName << "doesn't exists";
    return nullptr;
}

DeviceList BioDevices::findDeviceByType(int nBioType)
{
    DeviceList listDevice;
    for (auto deviceInfo : deviceInfos) {
        if (deviceInfo->biotype == nBioType) {
            listDevice.append(deviceInfo);
        }
    }
    return listDevice;
}

DeviceInfoPtr BioDevices::getFirstDevice(int uid)
{
    for (DeviceInfoPtr devInfo : deviceInfos) {
        if (devInfo && GetUserDevFeatureCount(uid, devInfo->device_id) > 0) {
            return devInfo;
        }
    }

    return nullptr;
}

QString BioDevices::bioTypeToString_tr(int type)
{
    switch (type) {
        case BIOTYPE_FINGERPRINT:
            return tr("FingerPrint");
        case BIOTYPE_FINGERVEIN:
            return tr("FingerVein");
        case BIOTYPE_IRIS:
            return tr("Iris");
        case BIOTYPE_FACE:
            return tr("Face");
        case BIOTYPE_VOICEPRINT:
            return tr("VoicePrint");
        case LOGINOPT_TYPE_GENERAL_UKEY:
            return tr("Ukey");
        case REMOTE_QRCODE_TYPE:
            return tr("QRCode");
    }
    return QString();
}

DeviceList BioDevices::GetDevList()
{
    return deviceInfos;
}

StatusReslut BioDevices::UpdateStatus(int drvid)
{
    StatusReslut status;
    QDBusMessage result = serviceInterface->call(QStringLiteral("UpdateStatus"), drvid);
    if (result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "UpdateStatus error:" << result.errorMessage();
        status.result = -1;
        return status;
    }

    status.result = result.arguments().at(0).toInt();
    status.enable = result.arguments().at(1).toInt();
    status.devNum = result.arguments().at(2).toInt();
    status.devStatus = result.arguments().at(3).toInt();
    status.opsStatus = result.arguments().at(4).toInt();
    status.notifyMessageId = result.arguments().at(5).toInt();

    return status;
}

// 这里用读取配置文件的方式替代原本调用dbus接口的方式，解决
// 开机后首次提权，提权窗口响应慢的问题，因为kylin-mfa的进程
// 不是开机自启动，在polkit调用时拉起，导致polkit窗口显示变慢
QString BioDevices::getAuthMethod()
{
    QString configPath = "/etc/pwdless/default.conf";
    QFile file(configPath);
    if (!file.exists()) {
        return "";
    }
    QSettings settings(configPath, QSettings::IniFormat);
    // 检查[global]区块下mfaAuth
    QVariant mfaAuthVar = settings.value("global/mfaAuth", "false");
    if (mfaAuthVar.toString().toLower() != "true") {
        return "";
    }
    // 检查[mfa]区块下method
    QVariant methodVar = settings.value("mfa/method", "");
    QString method = methodVar.toString().trimmed();
    if (method.isEmpty()) {
        return "";
    }
    return method;
}
