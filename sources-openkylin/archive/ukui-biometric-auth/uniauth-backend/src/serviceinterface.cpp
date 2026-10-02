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
#include "serviceinterface.h"
#include "dbusaccesscontrol.h"
#include "usersafeconfigpath.h"
#include <QDBusConnection>
#include <QDir>
#include <QRegExp>
#include <QFile>
#include <QTimer>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <QFileSystemWatcher>
#include <QDebug>
#include <QProcess>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusError>
#include "servicemanager.h"
#include "personalizeddata.h"
#include <unistd.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <syslog.h>
#include "userconfigsettings.h"

#define UKUI_GREETER "/usr/sbin/ukui-greeter"
#define UKUI_SCREENSAVER "/usr/bin/ukui-screensaver-backend"

#define GREETER_DBUS_PATH "/"
#define GREETER_DBUS_INTERFACE "org.ukui.greeter"
#define SCREENSAVER_DBUS_PATH "/"
#define SCREENSAVER_DBUS_INTERFACE "org.ukui.screensaver"
#define COMM_CONFIG_PATH "/etc/biometric-auth/ukui-biometric.conf"
#define PROC_CPUINFO "/proc/cpuinfo"
#define LAST_LOGIN_INFO_PATH "/var/lib/lightdm/.cache/ukui-greeter.conf"

/* For the type WillLoginUserInfo */
QDBusArgument &operator<<(QDBusArgument &argument, const WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WillLoginUserInfo &willLoginUserInfo)
{
    argument.beginStructure();
    argument >> willLoginUserInfo.strUserName >> willLoginUserInfo.isOneKeyLogin;
    argument.endStructure();
    return argument;
}

QDebug operator<<(QDebug stream, const WillLoginUserInfo &willLoginUserInfo)
{
    stream << "WillLogUser [" << willLoginUserInfo.strUserName << willLoginUserInfo.isOneKeyLogin << "]";
    return stream;
}

ServiceInterface::ServiceInterface()
{
    syslog(LOG_INFO, "[uniauth-backend] ServiceInterface init begin");
    bool res = QDBusConnection::systemBus().registerService("org.ukui.UniauthBackend");
    if (!res) {
        qInfo() << "registerService org.ukui.UniauthBackend failed!!";
        exit(0);
    }
    res = QDBusConnection::systemBus().registerObject(
        "/org/ukui/UniauthBackend",
        "org.ukui.UniauthBackend",
        this,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    if (!res) {
        qInfo() << "registerObject /org/ukui/UniauthBackend failed!!";
        exit(0);
    }

    m_greeterConfigInterface = new GreeterConfigInterface(this);
    res = QDBusConnection::systemBus().registerObject(
        "/org/ukui/GreeterConfig",
        "org.ukui.UniauthBackend",
        m_greeterConfigInterface,
        QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    if (!res) {
        qInfo() << "registerObject /org/ukui/GreeterConfig failed!!";
    }

    registerCustomTypes();
    qRegisterMetaType<WillLoginUserInfo>("WillLoginUserInfo");
    qDBusRegisterMetaType<WillLoginUserInfo>();
    checkIs990();
    m_serviceInterface
        = new QDBusInterface(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, QDBusConnection::systemBus(), this);

    m_serviceInterface->setTimeout(2147483647);
    connect(m_serviceInterface, SIGNAL(USBDeviceHotPlug(int, int, int)), this, SLOT(onUSBDeviceHotPlug(int, int, int)));
    ServiceManager *sm = ServiceManager::instance();
    connect(sm, &ServiceManager::serviceStatusChanged, this, &ServiceInterface::onBiometricDbusChanged);
    connect(
        KYLINUSERDATAMNG::getInstance(),
        &PersonalizedDataMng::conf_changed,
        this,
        &ServiceInterface::updateUserInformation);
    syslog(LOG_INFO, "[uniauth-backend] ServiceInterface init end");
}

bool ServiceInterface::ensureTrustedCaller()
{
    if (UniauthDBusAccess::isCallerRoot(connection(), message())
        || UniauthDBusAccess::isCallerProcessWhitelisted(connection(), message())) {
        return true;
    }
    sendErrorReply(QDBusError::AccessDenied, QStringLiteral("Access denied"));
    return false;
}

bool ServiceInterface::ensureRootCaller()
{
    if (UniauthDBusAccess::isCallerRoot(connection(), message())) {
        return true;
    }
    sendErrorReply(QDBusError::AccessDenied, QStringLiteral("Access denied"));
    return false;
}

void ServiceInterface::setDefaultDevice(QString userName, int bioDevType, QString deviceName)
{
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "setDefaultDevice: unsafe or invalid config path for user" << userName;
        return;
    }
    qDebug() << configPath << bioDevType;
    UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
    switch (bioDevType) {
        case BIOTYPE_FACE:
            settings.setValue("FC_DefaultDevice", deviceName);
            break;
        case BIOTYPE_FINGERPRINT:
            settings.setValue("FP_DefaultDevice", deviceName);
            break;
        case BIOTYPE_FINGERVEIN:
            settings.setValue("FV_DefaultDevice", deviceName);
            break;
        case BIOTYPE_IRIS:
            settings.setValue("IR_DefaultDevice", deviceName);
            break;
        case BIOTYPE_VOICEPRINT:
            settings.setValue("VP_DefaultDevice", deviceName);
            break;
        case UNIT_GENERAL_UKEY:
            settings.setValue("GU_DefaultDevice", deviceName);
            break;
        case REMOTE_QRCODE_TYPE:
            settings.setValue("WC_DefaultDevice", deviceName);
            break;
        default:
            break;
    }
    settings.sync();
    qDebug() << "setDefaultDevice:" << userName << "," << bioDevType << "," << deviceName;
    emit defaultDeviceChanged(userName, bioDevType, deviceName);
}

void ServiceInterface::setDefaultDevice(int bioDevType, QString deviceName)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    int uid = conn.interface()->serviceUid(msg.service()).value();
    struct passwd *pwinfo = getpwuid(uid);
    if (pwinfo && pwinfo->pw_name) {
        setDefaultDevice(pwinfo->pw_name, bioDevType, deviceName);
    } else {
        qInfo() << "GetPWInfo failed!!";
    }
}

QString ServiceInterface::getDefaultDevice(QString userName, int bioDevType)
{
    if (!ensureTrustedCaller()) {
        return QString();
    }
    initData();
    QString defaultDevice = "";
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "getDefaultDevice: unsafe or invalid config path for user" << userName;
        QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
        switch (bioDevType) {
            case BIOTYPE_FACE:
                defaultDevice = settings2.value("FC_DefaultDevice").toString();
                break;
            case BIOTYPE_FINGERPRINT:
                defaultDevice = settings2.value("FP_DefaultDevice").toString();
                break;
            case BIOTYPE_FINGERVEIN:
                defaultDevice = settings2.value("FV_DefaultDevice").toString();
                break;
            case BIOTYPE_IRIS:
                defaultDevice = settings2.value("IR_DefaultDevice").toString();
                break;
            case BIOTYPE_VOICEPRINT:
                defaultDevice = settings2.value("VP_DefaultDevice").toString();
                break;
            case UNIT_GENERAL_UKEY:
                defaultDevice = settings2.value("GU_DefaultDevice").toString();
                break;
            case REMOTE_QRCODE_TYPE:
                defaultDevice = settings2.value("WC_DefaultDevice").toString();
                break;
            default:
                defaultDevice = settings2.value("DefaultDevice").toString();
                break;
        }
    } else {
        UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
        // 获取用户旧的默认设备
        if (settings.contains("DefaultDevice")) {
            QString strOldDefDev = settings.value("DefaultDevice").toString();
            if (!strOldDefDev.isEmpty()) {
                for (auto devInfo : m_listDeviceInfos) {
                    if (devInfo && devInfo->device_shortname == strOldDefDev) {
                        QString strBioDefType = "";
                        switch (devInfo->biotype) {
                            case BIOTYPE_FINGERPRINT:
                                strBioDefType = "FP_DefaultDevice";
                                break;
                            case BIOTYPE_FINGERVEIN:
                                strBioDefType = "FV_DefaultDevice";
                                break;
                            case BIOTYPE_IRIS:
                                strBioDefType = "IR_DefaultDevice";
                                break;
                            case BIOTYPE_FACE:
                                strBioDefType = "FC_DefaultDevice";
                                break;
                            case BIOTYPE_VOICEPRINT:
                                strBioDefType = "VP_DefaultDevice";
                                break;
                            case UNIT_GENERAL_UKEY:
                                strBioDefType = "GU_DefaultDevice";
                                break;
                            case REMOTE_QRCODE_TYPE:
                                strBioDefType = "WC_DefaultDevice";
                                break;
                            default:
                                break;
                        }
                        if (!strBioDefType.isEmpty() && !settings.contains(strBioDefType)) {
                            settings.setValue(strBioDefType, strOldDefDev);
                            settings.setValue("DefaultDevice", "");
                            settings.sync();
                        }
                        if (bioDevType == devInfo->biotype) {
                            return strOldDefDev;
                        }
                        break;
                    }
                }
            }
        }
        switch (bioDevType) {
            case BIOTYPE_FACE:
                defaultDevice = settings.value("FC_DefaultDevice").toString();
                break;
            case BIOTYPE_FINGERPRINT:
                defaultDevice = settings.value("FP_DefaultDevice").toString();
                break;
            case BIOTYPE_FINGERVEIN:
                defaultDevice = settings.value("FV_DefaultDevice").toString();
                break;
            case BIOTYPE_IRIS:
                defaultDevice = settings.value("IR_DefaultDevice").toString();
                break;
            case BIOTYPE_VOICEPRINT:
                defaultDevice = settings.value("VP_DefaultDevice").toString();
                break;
            case UNIT_GENERAL_UKEY:
                defaultDevice = settings.value("GU_DefaultDevice").toString();
                break;
            case REMOTE_QRCODE_TYPE:
                defaultDevice = settings.value("WC_DefaultDevice").toString();
                break;
            default:
                defaultDevice = settings.value("DefaultDevice").toString();
                break;
        }
        if (defaultDevice.isEmpty()) {
            QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
            switch (bioDevType) {
                case BIOTYPE_FACE:
                    defaultDevice = settings2.value("FC_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERPRINT:
                    defaultDevice = settings2.value("FP_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERVEIN:
                    defaultDevice = settings2.value("FV_DefaultDevice").toString();
                    break;
                case BIOTYPE_IRIS:
                    defaultDevice = settings2.value("IR_DefaultDevice").toString();
                    break;
                case BIOTYPE_VOICEPRINT:
                    defaultDevice = settings2.value("VP_DefaultDevice").toString();
                    break;
                case UNIT_GENERAL_UKEY:
                    defaultDevice = settings2.value("GU_DefaultDevice").toString();
                    break;
                case REMOTE_QRCODE_TYPE:
                    defaultDevice = settings2.value("WC_DefaultDevice").toString();
                    break;
                default:
                    defaultDevice = settings2.value("DefaultDevice").toString();
                    break;
            }
        }
    }

    return defaultDevice;
}

void ServiceInterface::setDefaultAuthType(QString userName, int bioDevType)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "setDefaultAuthType: unsafe or invalid config path for user" << userName;
        return;
    }
    UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
    settings.setValue("DefaultAuthType", bioDevType);
    settings.sync();
    qDebug() << "setDefaultAuthType:" << userName << "," << bioDevType;
    emit defaultAuthTypeChanged(userName, bioDevType);
}

int ServiceInterface::getDefaultAuthType(QString userName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    int bioDevType;
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "getDefaultDevice: unsafe or invalid config path for user" << userName;
        QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
        bioDevType = settings2.value("DefaultAuthType").toInt();
        settings2.sync();
    } else {
        UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
        bioDevType = settings.value("DefaultAuthType").toInt();
        settings.sync();
    }
    return bioDevType;
}

QStringList ServiceInterface::getAllDefaultDevice(QString userName)
{
    if (!ensureTrustedCaller()) {
        return QStringList();
    }
    initData();
    QStringList listDefDevice;
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "getAllDefaultDevice: unsafe or invalid config path for user" << userName;
        QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
        for (int nBioType = 0; nBioType <= REMOTE_QRCODE_TYPE; nBioType++) {
            QString defaultDevice = "";
            switch (nBioType) {
                case BIOTYPE_FACE:
                    defaultDevice = settings2.value("FC_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERPRINT:
                    defaultDevice = settings2.value("FP_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERVEIN:
                    defaultDevice = settings2.value("FV_DefaultDevice").toString();
                    break;
                case BIOTYPE_IRIS:
                    defaultDevice = settings2.value("IR_DefaultDevice").toString();
                    break;
                case BIOTYPE_VOICEPRINT:
                    defaultDevice = settings2.value("VP_DefaultDevice").toString();
                    break;
                case UNIT_GENERAL_UKEY:
                    defaultDevice = settings2.value("GU_DefaultDevice").toString();
                    break;
                case REMOTE_QRCODE_TYPE:
                    defaultDevice = settings2.value("WC_DefaultDevice").toString();
                    break;
                default:
                    break;
            }
            if (!defaultDevice.isEmpty()) {
                listDefDevice.push_back(defaultDevice);
            }
        }
    } else {
        UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
        int nOldDefType = -1;
        // 获取用户旧的默认设备
        if (settings.contains("DefaultDevice")) {
            QString strOldDefDev = settings.value("DefaultDevice").toString();
            if (!strOldDefDev.isEmpty()) {
                for (auto devInfo : m_listDeviceInfos) {
                    if (devInfo && devInfo->device_shortname == strOldDefDev) {
                        QString strBioDefType = "";
                        switch (devInfo->biotype) {
                            case BIOTYPE_FINGERPRINT:
                                strBioDefType = "FP_DefaultDevice";
                                break;
                            case BIOTYPE_FINGERVEIN:
                                strBioDefType = "FV_DefaultDevice";
                                break;
                            case BIOTYPE_IRIS:
                                strBioDefType = "IR_DefaultDevice";
                                break;
                            case BIOTYPE_FACE:
                                strBioDefType = "FC_DefaultDevice";
                                break;
                            case BIOTYPE_VOICEPRINT:
                                strBioDefType = "VP_DefaultDevice";
                                break;
                            case UNIT_GENERAL_UKEY:
                                strBioDefType = "GU_DefaultDevice";
                                break;
                            case REMOTE_QRCODE_TYPE:
                                strBioDefType = "WC_DefaultDevice";
                                break;
                            default:
                                break;
                        }
                        if (!strBioDefType.isEmpty() && !settings.contains(strBioDefType)) {
                            nOldDefType = devInfo->biotype;
                            listDefDevice.push_back(strOldDefDev);
                            settings.setValue(strBioDefType, strOldDefDev);
                            settings.setValue("DefaultDevice", "");
                            settings.sync();
                        }
                        break;
                    }
                }
            }
        }
        QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
        for (int nBioType = 0; nBioType <= REMOTE_QRCODE_TYPE; nBioType++) {
            QString defaultDevice = "";
            if (nBioType == nOldDefType)
                continue;
            switch (nBioType) {
                case BIOTYPE_FACE:
                    defaultDevice = settings.value("FC_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERPRINT:
                    defaultDevice = settings.value("FP_DefaultDevice").toString();
                    break;
                case BIOTYPE_FINGERVEIN:
                    defaultDevice = settings.value("FV_DefaultDevice").toString();
                    break;
                case BIOTYPE_IRIS:
                    defaultDevice = settings.value("IR_DefaultDevice").toString();
                    break;
                case BIOTYPE_VOICEPRINT:
                    defaultDevice = settings.value("VP_DefaultDevice").toString();
                    break;
                case UNIT_GENERAL_UKEY:
                    defaultDevice = settings.value("GU_DefaultDevice").toString();
                    break;
                case REMOTE_QRCODE_TYPE:
                    defaultDevice = settings.value("WC_DefaultDevice").toString();
                    break;
                default:
                    break;
            }
            if (defaultDevice.isEmpty()) {
                switch (nBioType) {
                    case BIOTYPE_FACE:
                        defaultDevice = settings2.value("FC_DefaultDevice").toString();
                        break;
                    case BIOTYPE_FINGERPRINT:
                        defaultDevice = settings2.value("FP_DefaultDevice").toString();
                        break;
                    case BIOTYPE_FINGERVEIN:
                        defaultDevice = settings2.value("FV_DefaultDevice").toString();
                        break;
                    case BIOTYPE_IRIS:
                        defaultDevice = settings2.value("IR_DefaultDevice").toString();
                        break;
                    case BIOTYPE_VOICEPRINT:
                        defaultDevice = settings2.value("VP_DefaultDevice").toString();
                        break;
                    case UNIT_GENERAL_UKEY:
                        defaultDevice = settings2.value("GU_DefaultDevice").toString();
                        break;
                    case REMOTE_QRCODE_TYPE:
                        defaultDevice = settings2.value("WC_DefaultDevice").toString();
                        break;
                    default:
                        break;
                }
            }
            if (!defaultDevice.isEmpty()) {
                listDefDevice.push_back(defaultDevice);
            }
        }
    }
    return listDefDevice;
}

// 设置通用默认设备
void ServiceInterface::setCommDefaultDevice(int bioDevType, QString deviceName)
{
    initData();
    QSettings settings(COMM_CONFIG_PATH, QSettings::IniFormat);
    switch (bioDevType) {
        case BIOTYPE_FACE:
            settings.setValue("FC_DefaultDevice", deviceName);
            break;
        case BIOTYPE_FINGERPRINT:
            settings.setValue("FP_DefaultDevice", deviceName);
            break;
        case BIOTYPE_FINGERVEIN:
            settings.setValue("FV_DefaultDevice", deviceName);
            break;
        case BIOTYPE_IRIS:
            settings.setValue("IR_DefaultDevice", deviceName);
            break;
        case BIOTYPE_VOICEPRINT:
            settings.setValue("VP_DefaultDevice", deviceName);
            break;
        case UNIT_GENERAL_UKEY:
            settings.setValue("GU_DefaultDevice", deviceName);
            break;
        case REMOTE_QRCODE_TYPE:
            settings.setValue("WC_DefaultDevice", deviceName);
            break;
        default:
            break;
    }
    settings.sync();
    qDebug() << "setCommDefaultDevice:" << bioDevType << "," << deviceName;
    emit defaultDeviceChanged("", bioDevType, deviceName);
}

// 获取通用默认设备
QString ServiceInterface::getCommDefaultDevice(int bioDevType)
{
    initData();
    QString defaultDevice = "";
    QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
    switch (bioDevType) {
        case BIOTYPE_FACE:
            defaultDevice = settings2.value("FC_DefaultDevice").toString();
            break;
        case BIOTYPE_FINGERPRINT:
            defaultDevice = settings2.value("FP_DefaultDevice").toString();
            break;
        case BIOTYPE_FINGERVEIN:
            defaultDevice = settings2.value("FV_DefaultDevice").toString();
            break;
        case BIOTYPE_IRIS:
            defaultDevice = settings2.value("IR_DefaultDevice").toString();
            break;
        case BIOTYPE_VOICEPRINT:
            defaultDevice = settings2.value("VP_DefaultDevice").toString();
            break;
        case UNIT_GENERAL_UKEY:
            defaultDevice = settings2.value("GU_DefaultDevice").toString();
            break;
        case REMOTE_QRCODE_TYPE:
            defaultDevice = settings2.value("WC_DefaultDevice").toString();
            break;
        default:
            defaultDevice = settings2.value("DefaultDevice").toString();
            break;
    }
    return defaultDevice;
}

void ServiceInterface::setBioAuthStatus(int bioAuthType, bool status)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    int uid = conn.interface()->serviceUid(msg.service()).value();
    struct passwd *pwinfo = getpwuid(uid);
    if (pwinfo && pwinfo->pw_name) {
        const QString configPath = safeUserBiometricConfPath(QString::fromUtf8(pwinfo->pw_name));
        if (configPath.isEmpty()) {
            qWarning() << "setBioAuthStatus: unsafe or invalid config path for user" << pwinfo->pw_name;
            return;
        }
        UserConfigSettings settings(configPath, QSettings::IniFormat, QString(pwinfo->pw_name));
        switch (bioAuthType) {
            case ENABLETYPE_BIO:
                settings.setValue("EnableAuth", status);
                break;
            case ENABLETYPE_SAVER:
                settings.setValue("SaverEnable", status);
                break;
            case ENABLETYPE_GREETER:
                settings.setValue("GreeterEnable", status);
                break;
            case ENABLETYPE_POLKIT:
                settings.setValue("PolkitEnable", status);
                break;
            case ENABLETYPE_SU:
                settings.setValue("SuEnable", status);
                break;
            case ENABLETYPE_SUDO:
                settings.setValue("SudoEnable", status);
                break;
            case ENABLETYPE_LOGIN:
                settings.setValue("LoginEnable", status);
                break;
            default:
                break;
        }
        settings.sync();
        emit bioAuthStatusChanged(pwinfo->pw_name, bioAuthType, status);
        qDebug() << "setBioAuthStatus:" << pwinfo->pw_name << "," << bioAuthType << "," << status;
    } else {
        qInfo() << "GetPWInfo failed!!";
    }
}

bool ServiceInterface::getBioAuthStatus(QString userName, int bioAuthType)
{
    if (!ensureTrustedCaller()) {
        return false;
    }

    initData();
    QString strBioAuthType = "";
    switch (bioAuthType) {
        case ENABLETYPE_BIO:
            strBioAuthType = "EnableAuth";
            break;
        case ENABLETYPE_SAVER:
            strBioAuthType = "SaverEnable";
            break;
        case ENABLETYPE_GREETER:
            strBioAuthType = "GreeterEnable";
            break;
        case ENABLETYPE_POLKIT:
            strBioAuthType = "PolkitEnable";
            break;
        case ENABLETYPE_SU:
            strBioAuthType = "SuEnable";
            break;
        case ENABLETYPE_SUDO:
            strBioAuthType = "SudoEnable";
            break;
        case ENABLETYPE_LOGIN:
            strBioAuthType = "LoginEnable";
            break;
        default:
            return false;
    }
    const QString configPath = safeUserBiometricConfPath(userName);
    if (configPath.isEmpty()) {
        qWarning() << "getBioAuthStatus: unsafe or invalid config path for user" << userName;
        if (getIs990() && (bioAuthType == ENABLETYPE_BIO)) {
            // 990默认为打开
            return true;
        } else {
            QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
            if (settings2.contains(strBioAuthType)) {
                return settings2.value(strBioAuthType).toBool();
            } else {
                return true;
            }
        }
    } else {
        UserConfigSettings settings(configPath, QSettings::IniFormat, userName);
        // 同步990旧的配置文件，然后删除
        if (getIs990() && (bioAuthType == ENABLETYPE_BIO)) {
            const QString oldConfigPath = safeUserLegacy990BiometricConfPath(userName);
            if (!oldConfigPath.isEmpty()) {
                QSettings oldSettings(oldConfigPath, QSettings::IniFormat);
                if (oldSettings.contains(strBioAuthType)) {
                    settings.setValue(strBioAuthType, oldSettings.value(strBioAuthType).toBool());
                    oldSettings.remove(strBioAuthType);
                    oldSettings.sync();
                }
            }
        }
        if (settings.contains(strBioAuthType)) {
            return settings.value(strBioAuthType).toBool();
        } else if (getIs990() && (bioAuthType == ENABLETYPE_BIO)) {
            // 990默认为打开
            return true;
        } else {
            QSettings settings2(COMM_CONFIG_PATH, QSettings::IniFormat);
            if (settings2.contains(strBioAuthType)) {
                return settings2.value(strBioAuthType).toBool();
            } else {
                return true;
            }
        }
    }
}

int ServiceInterface::getMaxFailedTimes()
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("MaxFailedTimes"))
        return sysSettings.value("MaxFailedTimes").toInt();
    else
        return 3;
}

int ServiceInterface::getFTimeoutTimes()
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("FaceTimeoutTimes"))
        return sysSettings.value("FaceTimeoutTimes").toInt();
    else
        return 1;
}

void ServiceInterface::setFTimeoutTimes(int times)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    if (times < 1)
        times = 1;
    QSettings settings(COMM_CONFIG_PATH, QSettings::IniFormat);
    settings.setValue("FaceTimeoutTimes", times);
}

bool ServiceInterface::getQRCodeEnable()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    bool isEnable = false;
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    sysSettings.beginGroup("Functions");
    if (sysSettings.allKeys().contains("EnableQRCode")) {
        isEnable = sysSettings.value("EnableQRCode").toBool();
    }
    sysSettings.endGroup();
    return isEnable;
}

// 获取是否双认证
bool ServiceInterface::getDoubleAuth()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("DoubleAuth"))
        return sysSettings.value("DoubleAuth").toBool();
    else
        return false;
}

// 获取用户绑定
bool ServiceInterface::getUserBind()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("UserBind"))
        return sysSettings.value("UserBind").toBool();
    else
        return false;
}

// 获取是否在控制面板显示
bool ServiceInterface::getIsShownInControlCenter()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("isShownInControlCenter"))
        return sysSettings.value("isShownInControlCenter").toBool();
    else
        return false;
}

// 获取是否使用第一个设备
bool ServiceInterface::getUseFirstDevice()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("UseFirstDevice"))
        return sysSettings.value("UseFirstDevice").toBool();
    else
        return false;
}

// 获取是否隐藏切换按钮
bool ServiceInterface::getHiddenSwitchButton()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("HiddenSwitchButton"))
        return sysSettings.value("HiddenSwitchButton").toBool();
    else
        return false;
}

// 获取旧版app使能值
int ServiceInterface::getOldAppStatus()
{
    initData();
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("EnableAuthApp"))
        return sysSettings.value("EnableAuthApp").toInt();
    else
        return 63;
}

void ServiceInterface::onUSBDeviceHotPlug(int drvid, int action, int deviceNum)
{
    if (action > 0) {
        qDebug() << "onUSBDeviceHotPlug in:" << drvid;
        updateCommDefaultDevice(drvid);
    }
}

// 记录当前生物认证信息
void ServiceInterface::recordAuthDrive(QString appName, int drvid, bool insert)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    if (insert) {
        m_AuthingDriveList.insert(appName, drvid);
    } else {
        m_AuthingDriveList.remove(appName);
    }
}

// 获取当前正在认证的应用
QStringList ServiceInterface::getAllAuthApp()
{
    if (!ensureTrustedCaller()) {
        return QStringList();
    }
    return m_AuthingDriveList.keys();
}

// 根据应用名称获取认证的驱动
int ServiceInterface::getAppAuthDrive(QString appName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    if (m_AuthingDriveList.contains(appName)) {
        return m_AuthingDriveList.value(appName);
    }
    return -1;
}

QString ServiceInterface::GetUserInformation(QString username)
{
    if (!ensureTrustedCaller()) {
        return QString();
    }
    KyInfo() << username;
    return KYLINUSERDATAMNG::getInstance()->GetConfInformation(username);
}

void ServiceInterface::SetLoginSynInformation(QString val)
{
    if (!ensureTrustedCaller()) {
        return;
    }
    KyInfo() << val;
    KYLINUSERDATAMNG::getInstance()->SetLoginSynInformation(val);
}

void ServiceInterface::waitBiometricServiceStatus()
{
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~` get biometric status";
    QDBusInterface iface(
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> bioPath = iface.call("GetUnit", "biometric-authentication.service");
    if (!bioPath.isValid()) {
        return;
    }

    QDBusInterface bioface(
        "org.freedesktop.systemd1",
        bioPath.value().path(),
        "org.freedesktop.DBus.Properties",
        QDBusConnection::systemBus());
    QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "UnitFileState");
    if (!sessionReply.isValid())
        qWarning() << sessionReply.error();
    else {
        QString res = sessionReply.value().variant().toString();
        if (res == "disable")
            return;
    }

    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   get activeState";
    int times = 0;
    while (times < 20) {
        QDBusReply<QDBusVariant> sessionReply = bioface.call("Get", "org.freedesktop.systemd1.Unit", "ActiveState");
        if (!sessionReply.isValid()) {
            qWarning() << sessionReply.error();
            return;
        } else {
            QString res = sessionReply.value().variant().toString();
            if (res == "activating") {
                times++;
                usleep(100000);
            } else {
                break;
            }
        }
    }
    qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ times = " << times;
}

void ServiceInterface::updateCommDefaultDevice(int nDriId)
{
    if (!m_serviceInterface) {
        return;
    }
    QVariant variant;
    QDBusArgument argument;
    QList<QDBusVariant> qlist;
    QDBusVariant item;
    DeviceInfoPtr deviceInfo = nullptr;

    /* 返回值为 i -- int 和 av -- array of variant */
    QDBusPendingReply<int, QList<QDBusVariant>> reply = m_serviceInterface->call("GetDrvList");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "GUI:" << reply.error();
        deviceCount = 0;
        return;
    }

    /* 解析 DBus 返回值，reply 有两个返回值，都是 QVariant 类型 */
    variant = reply.argumentAt(0);             /* 得到第一个返回值 */
    deviceCount = variant.value<int>();        /* 解封装得到设备个数 */
    variant = reply.argumentAt(1);             /* 得到第二个返回值 */
    argument = variant.value<QDBusArgument>(); /* 解封装，获取QDBusArgument对象 */
    argument >> qlist; /* 使用运算符重载提取 argument 对象里面存储的列表对象 */

    if (nDriId != -1) { // 指定设备接入
        for (int i = 0; i < deviceCount; i++) {
            item = qlist[i];          /* 取出一个元素 */
            variant = item.variant(); /* 转为普通QVariant对象 */
            /* 解封装得到 QDBusArgument 对象 */
            argument = variant.value<QDBusArgument>();
            deviceInfo = std::make_shared<DeviceInfo>();
            argument >> *deviceInfo; /* 提取最终的 DeviceInfo 结构体 */
            if (nDriId == deviceInfo->device_id) {
                if (getCommDefaultDevice(deviceInfo->biotype).isEmpty()) {
                    setCommDefaultDevice(deviceInfo->biotype, deviceInfo->device_shortname);
                }
                break;
            }
        }
    } else {
        m_listDeviceInfos.clear();
        for (int i = 0; i < deviceCount; i++) {
            item = qlist[i];          /* 取出一个元素 */
            variant = item.variant(); /* 转为普通QVariant对象 */
            /* 解封装得到 QDBusArgument 对象 */
            argument = variant.value<QDBusArgument>();
            deviceInfo = std::make_shared<DeviceInfo>();
            argument >> *deviceInfo; /* 提取最终的 DeviceInfo 结构体 */
            m_listDeviceInfos.push_back(deviceInfo);
        }
    }
}

void ServiceInterface::initData()
{
    if (m_isDataInited) {
        return;
    }
    updateCommDefaultDevice(-1);
    // 沿用旧app enable
    QSettings sysSettings(COMM_CONFIG_PATH, QSettings::IniFormat);
    if (sysSettings.contains("EnableAuthApp") || sysSettings.contains("Functions/EnableAuthApp")) {
        int nAppOldStatus = 0;
        if (sysSettings.contains("Functions/EnableAuthApp")) {
            nAppOldStatus = sysSettings.value("Functions/EnableAuthApp").toInt();
        } else {
            nAppOldStatus = sysSettings.value("EnableAuthApp").toInt();
        }
        if ((nAppOldStatus & 0x01) && !sysSettings.contains("GreeterEnable")) { // greeter
            sysSettings.setValue("GreeterEnable", true);
        }
        if ((nAppOldStatus & 0x02) && !sysSettings.contains("SaverEnable")) { // saver
            sysSettings.setValue("SaverEnable", true);
        }
        if ((nAppOldStatus & 0x04) && !sysSettings.contains("PolkitEnable")) { // polkit
            sysSettings.setValue("PolkitEnable", true);
        }
        if ((nAppOldStatus & 0x08) && !sysSettings.contains("SudoEnable")) { // sudo
            sysSettings.setValue("SudoEnable", true);
        }
        if ((nAppOldStatus & 0x10) && !sysSettings.contains("SuEnable")) { // su
            sysSettings.setValue("SuEnable", true);
        }
        if ((nAppOldStatus & 0x20) && !sysSettings.contains("LoginEnable")) { // login
            sysSettings.setValue("LoginEnable", true);
        }
    }
    // 沿用旧的默认设备
    if (sysSettings.contains("DefaultDevice")) {
        QString strOldDefDev = sysSettings.value("DefaultDevice").toString();
        if (!strOldDefDev.isEmpty()) {
            for (auto devInfo : m_listDeviceInfos) {
                if (devInfo && devInfo->device_shortname == strOldDefDev) {
                    QString strBioDefType = "";
                    switch (devInfo->biotype) {
                        case BIOTYPE_FINGERPRINT:
                            strBioDefType = "FP_DefaultDevice";
                            break;
                        case BIOTYPE_FINGERVEIN:
                            strBioDefType = "FV_DefaultDevice";
                            break;
                        case BIOTYPE_IRIS:
                            strBioDefType = "IR_DefaultDevice";
                            break;
                        case BIOTYPE_FACE:
                            strBioDefType = "FC_DefaultDevice";
                            break;
                        case BIOTYPE_VOICEPRINT:
                            strBioDefType = "VP_DefaultDevice";
                            break;
                        case UNIT_GENERAL_UKEY:
                            strBioDefType = "GU_DefaultDevice";
                            break;
                        case REMOTE_QRCODE_TYPE:
                            strBioDefType = "WC_DefaultDevice";
                            break;
                        default:
                            break;
                    }
                    if (!strBioDefType.isEmpty() && !sysSettings.contains(strBioDefType)) {
                        sysSettings.setValue(strBioDefType, strOldDefDev);
                        sysSettings.setValue("DefaultDevice", "");
                    }
                    break;
                }
            }
        }
    }
    // 设置未设置过默认设备的类型，如果驱动已打开且已连接
    for (auto devInfo : m_listDeviceInfos) {
        if (devInfo && devInfo->device_available > 0 && devInfo->driver_enable > 0) {
            QString strBioDefType = "";
            switch (devInfo->biotype) {
                case BIOTYPE_FINGERPRINT:
                    strBioDefType = "FP_DefaultDevice";
                    break;
                case BIOTYPE_FINGERVEIN:
                    strBioDefType = "FV_DefaultDevice";
                    break;
                case BIOTYPE_IRIS:
                    strBioDefType = "IR_DefaultDevice";
                    break;
                case BIOTYPE_FACE:
                    strBioDefType = "FC_DefaultDevice";
                    break;
                case BIOTYPE_VOICEPRINT:
                    strBioDefType = "VP_DefaultDevice";
                    break;
                case UNIT_GENERAL_UKEY:
                    strBioDefType = "GU_DefaultDevice";
                    break;
                case REMOTE_QRCODE_TYPE:
                    strBioDefType = "WC_DefaultDevice";
                    break;
                default:
                    break;
            }
            if (!strBioDefType.isEmpty() && !sysSettings.contains(strBioDefType)) {
                sysSettings.setValue(strBioDefType, devInfo->device_shortname);
            }
        }
    }
    sysSettings.sync();
    m_isDataInited = true;
}

void ServiceInterface::onBiometricDbusChanged(bool bActive)
{
    qDebug() << "BiometricDbus:" << bActive;
    if (bActive) {
        updateCommDefaultDevice(-1);
        initData();
    }
}

void ServiceInterface::checkIs990()
{
    QRegExp r1("kirin.*9.0");
    r1.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp r2("pangu.*m900");
    r2.setCaseSensitivity(Qt::CaseInsensitive);

    QFile file(PROC_CPUINFO);
    if (!file.exists()) {
        is990 = false;
    }
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    is990 = (str.contains(r1) || str.contains(r2));
    file.close();
}

bool ServiceInterface::getIs990()
{
    return is990;
}

QString ServiceInterface::getPublicEncrypt()
{
    if (!ensureTrustedCaller()) {
        return QString();
    }
    if (pubKey.isEmpty()) {
        rsac.generateKeyPair(priKey, pubKey, 1024);
    }
    return pubKey;
}

bool ServiceInterface::sendPassword(QString username, QByteArray password)
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    if (priKey.isEmpty()) {
        rsac.generateKeyPair(priKey, pubKey, 1024);
    }
    QByteArray decryptText;
    QByteArray encryptText;
    rsac.decrypt(password, decryptText, priKey); // 解密
    // 如果处于登录状态

    qDebug() << "sendPassword" << loginAppList.count() << loginAppList;
    if (loginAppList.contains("lightdm")) {
        qDebug() << "login:";
        isFirstLogin = false;
        QString publicKey
            = getLoginPubKey(loginAppList.value("lightdm"), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE);
        rsac.encrypt(decryptText, encryptText, publicKey.toLatin1());
        return sendLoginPassword(
            loginAppList.value("lightdm"), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE, username, encryptText);
    }

    // 处于锁屏状态
    if (loginAppList.contains(username)) {
        qDebug() << "unlock:";
        isFirstLogin = false;
        QString publicKey
            = getLoginPubKey(loginAppList.value(username), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE);
        rsac.encrypt(decryptText, encryptText, publicKey.toLatin1());
        return sendLoginPassword(
            loginAppList.value(username), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE, username, encryptText);
    }

    if (loginAppList.count() > 0) {
        qDebug() << "login by screensaver:";
        isFirstLogin = false;
        QString publicKey = getLoginPubKey(
            loginAppList.value(loginAppList.firstKey()), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE);
        rsac.encrypt(decryptText, encryptText, publicKey.toLatin1());
        return sendLoginPassword(
            loginAppList.value(loginAppList.firstKey()),
            SCREENSAVER_DBUS_PATH,
            SCREENSAVER_DBUS_INTERFACE,
            username,
            encryptText);
    }

    // 如果是第一次调用，则配置自动登录
    if (isFirstLogin) {
        qDebug() << "isFirstLogin:";
        isFirstLogin = false;
        hasSaveLogin = true;
        return handleFirstSingleLogn(username, decryptText);
    }

    isFirstLogin = false;
    return false;
}

bool ServiceInterface::handleFirstSingleLogn(QString username, QByteArray decryptText)
{
    QEventLoop loopTemp;
    bool ret = false;
    QByteArray encryptText;

    qDebug() << "handleFirstSingleLogn wait";
    QMetaObject::Connection connection
        = QObject::connect(this, &ServiceInterface::onLoginServicdRegisted, [&loopTemp]() {
              if (loopTemp.isRunning()) {
                  loopTemp.quit();
              }
          });
    QTimer::singleShot(10000, &loopTemp, SLOT(quit()));
    loopTemp.exec();
    qDebug() << "handleFirstSingleLogn finished";
    disconnect(connection);
    qDebug() << loginAppList.count();
    if (loginAppList.contains("lightdm")) {
        isFirstLogin = false;
        QString publicKey
            = getLoginPubKey(loginAppList.value("lightdm"), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE);
        if (publicKey.isEmpty()) {
            qDebug() << "get Login PubKey failed!";
            return false;
        }
        rsac.encrypt(decryptText, encryptText, publicKey.toLatin1());
        return sendLoginPassword(
            loginAppList.value("lightdm"), SCREENSAVER_DBUS_PATH, SCREENSAVER_DBUS_INTERFACE, username, encryptText);
    }
    return ret;
}

QString ServiceInterface::getLoginPubKey(QString service, QString path, QString interface)
{
    QDBusInterface iface(service, path, interface, QDBusConnection::systemBus());
    QDBusReply<QString> stateReply = iface.call("getPublicEncrypt");

    if (!stateReply.isValid()) {
        qWarning() << "Get state error:" << stateReply.error();
        qDebug() << "getPublicEncrypt failed";
        return "";
    }
    return stateReply.value();
}

bool ServiceInterface::sendSignalLoginFinished(QString username, bool res)
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    qDebug() << "sendSignalLoginFinished " << username << res;
    emit onSignalLoginFinished(username, res);

    return true;
}

bool ServiceInterface::sendLoginPassword(
    QString service, QString path, QString interface, QString username, QByteArray array)
{
    QEventLoop loopTemp;
    bool ret = false;

    qDebug() << "sendLoginPassword";
    QMetaObject::Connection connection = QObject::connect(
        this, &ServiceInterface::onSignalLoginFinished, [&loopTemp, &ret](QString username, bool res) {
            qDebug() << "onSignalLoginFinished:" << username << res;
            ret = res;
            if (loopTemp.isRunning()) {
                loopTemp.quit();
            }
        });

    QDBusInterface iface(service, path, interface, QDBusConnection::systemBus());
    QDBusReply<bool> stateReply = iface.call("sendPassword", username, array);
    if (!stateReply.isValid()) {
        disconnect(connection);
        return false;
    }

    if (!stateReply.value()) {
        disconnect(connection);
        return stateReply.value();
    }

    QTimer::singleShot(30000, &loopTemp, SLOT(quit()));

    loopTemp.exec();
    disconnect(connection);

    return ret;
}

QString ServiceInterface::getProcessNameFromPid(int pid)
{
    QString filename = "/proc/" + QString::number(pid) + "/cmdline";
    qDebug() << "filename is " << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Can't open the file!";
        return "";
    }
    QByteArray text = file.readAll();
    return QString(text);
}

QString ServiceInterface::getUserNameFromUid(int uid)
{
    struct passwd *pw = NULL;
    pw = getpwuid(uid);
    if (pw) {
        return QString(pw->pw_name);
    }
    return "";
}

void ServiceInterface::onDbusNameOwnerChanged(QString name, QString oldOwner, QString newOwner)
{
    if (newOwner.isEmpty()) {
        for (QString str : loginAppList.keys()) {
            if (name == loginAppList.value(str)) {
                qDebug() << "remove name  " << name;
                loginAppList.remove(str);
            }
        }
    }
}

bool ServiceInterface::registerLoginApp()
{
    if (!ensureTrustedCaller()) {
        return false;
    }
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    qDebug() << "Sender Name" << conn.interface()->serviceOwner(msg.service()).value();
    qDebug() << "Sender Pid " << conn.interface()->servicePid(msg.service()).value();
    qDebug() << "Sender uid " << conn.interface()->serviceUid(msg.service()).value();

    int pid = conn.interface()->servicePid(msg.service()).value();
    QString serviceOwner = conn.interface()->serviceOwner(msg.service()).value();
    int uid = conn.interface()->serviceUid(msg.service()).value();

    QString appName = getProcessNameFromPid(pid);
    qDebug() << "appName = " << appName;

    if (!UniauthDBusAccess::isCallerRoot(conn, msg) && appName != UKUI_GREETER && appName != UKUI_SCREENSAVER) {
        return false;
    }

    QString appUser = getUserNameFromUid(uid);
    if (appUser.isEmpty())
        return false;

    qDebug() << "insert " << appUser << serviceOwner;
    loginAppList.insert(appUser, serviceOwner);

    if (appUser == "lightdm" && hasSaveLogin) {
        QTimer::singleShot(1000, this, [=]() { emit onLoginServicdRegisted(); });
    }

    return true;
}

int ServiceInterface::SwitchToUser(QString strUserName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    m_willLoginUser.strUserName = strUserName;
    m_willLoginUser.isOneKeyLogin = true;
    Q_EMIT userChanged(m_willLoginUser);
    return 0;
}

int ServiceInterface::SwitchToGreeterUser(QString strUserName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    m_willLoginUser.strUserName = strUserName;
    m_willLoginUser.isOneKeyLogin = false;
    Q_EMIT userChanged(m_willLoginUser);
    return 0;
}

WillLoginUserInfo ServiceInterface::getWillSwitchUser()
{
    if (!ensureTrustedCaller()) {
        return WillLoginUserInfo();
    }
    return m_willLoginUser;
}

int ServiceInterface::SaveLastLoginUser(QString strUserName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    UserConfigSettings settings(LAST_LOGIN_INFO_PATH, QSettings::IniFormat, "lightdm");
    settings.beginGroup("Greeter");
    settings.setValue("lastLoginUser", strUserName);
    settings.endGroup();
    settings.sync();
    return 0;
}

QString ServiceInterface::GetLastLoginUser()
{
    if (!ensureTrustedCaller()) {
        return QString();
    }
    UserConfigSettings settings(LAST_LOGIN_INFO_PATH, QSettings::IniFormat, "lightdm");
    settings.beginGroup("Greeter");
    QString lastLoginUser = settings.value("lastLoginUser").toString();
    settings.endGroup();
    return lastLoginUser;
}

int ServiceInterface::SaveQuickLoginUser(QString strUserName)
{
    if (!ensureTrustedCaller()) {
        return -1;
    }
    UserConfigSettings settings(LAST_LOGIN_INFO_PATH, QSettings::IniFormat, "lightdm");
    settings.beginGroup("Greeter");
    settings.setValue("lastLoginUser1", strUserName);
    settings.endGroup();
    settings.sync();
    return 0;
}

QString ServiceInterface::GetQuickLoginUser()
{
    if (!ensureTrustedCaller()) {
        return QString();
    }
    UserConfigSettings settings(LAST_LOGIN_INFO_PATH, QSettings::IniFormat, "lightdm");
    settings.beginGroup("Greeter");
    QString lastLoginUser = settings.value("lastLoginUser1").toString();
    settings.endGroup();
    return lastLoginUser;
}
