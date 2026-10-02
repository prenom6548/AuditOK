/**
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
#include "biometricdeviceinfo.h"
#include <QFile>

QString DeviceType::getDeviceType(int deviceType)
{
    if(deviceType >= __MAX_NR_TYPES)
    {
        return "";
    }
    QMetaEnum meta = QMetaEnum::fromType<Type>();
    const char *typeString = meta.valueToKey(deviceType);
    return QString(typeString);
}

QString DeviceType::getDeviceType_tr(int deviceType)
{
    switch(deviceType)
    {
    case FingerPrint:
        return tr("FingerPrint");
    case FingerVein:
        return tr("FingerVein");
    case Iris:
        return tr("Iris");
    case Face:
        return tr("Face");
    case VoicePrint:
        return tr("VoicePrint");
    case LOGINOPT_TYPE_GENERAL_UKEY:
        return tr("Ukey");
    case REMOTE_QRCODE_TYPE:
        return tr("QRCode");
    default:
        return "";
    }
}

int DeviceType::getBioType(int bioType)
{
    switch(bioType)
    {
    case Face:
        return 0;
    case FingerPrint:
        return 1;
    case FingerVein:
        return 2;
    case Iris:
        return 3;
    case VoicePrint:
        return 4;
    default:
        return -1;
    }
}

QDebug operator <<(QDebug stream, const DeviceInfo &deviceInfo)
{
    stream << "["
           << deviceInfo.id
           << deviceInfo.shortName
           << deviceInfo.fullName
           << deviceInfo.deviceType
           << deviceInfo.driverEnable
           << deviceInfo.deviceNum
           << "]";
    return stream;
}

QDBusArgument &operator <<(QDBusArgument &arg, const DeviceInfo &deviceInfo)
{
    arg.beginStructure();
    arg << deviceInfo.id
        << deviceInfo.shortName
        << deviceInfo.fullName
        << deviceInfo.driverEnable
        << deviceInfo.deviceNum
        << deviceInfo.deviceType
        << deviceInfo.storageType
        << deviceInfo.eigType
        << deviceInfo.verifyType
        << deviceInfo.identifyType
        << deviceInfo.busType
        << deviceInfo.deviceStatus
        << deviceInfo.OpsStatus;
    arg.endStructure();
    return arg;
}
const QDBusArgument &operator >>(const QDBusArgument &arg, DeviceInfo &deviceInfo)
{
    arg.beginStructure();
    arg >> deviceInfo.id
        >> deviceInfo.shortName
        >> deviceInfo.fullName
        >> deviceInfo.driverEnable
        >> deviceInfo.deviceNum
        >> deviceInfo.deviceType
        >> deviceInfo.storageType
        >> deviceInfo.eigType
        >> deviceInfo.verifyType
        >> deviceInfo.identifyType
        >> deviceInfo.busType
        >> deviceInfo.deviceStatus
        >> deviceInfo.OpsStatus;
    arg.endStructure();
    return arg;
}

/* For the type FeatureInfo */
QDBusArgument &operator<<(QDBusArgument &argument, const FeatureInfo &featureInfo)
{
    argument.beginStructure();
    argument << featureInfo.uid << featureInfo.biotype
        << featureInfo.device_shortname << featureInfo.index
        << featureInfo.index_name;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FeatureInfo &featureInfo)
{
    argument.beginStructure();
    argument >> featureInfo.uid >> featureInfo.biotype
        >> featureInfo.device_shortname >> featureInfo.index
        >> featureInfo.index_name;
    argument.endStructure();
    return argument;
}

void registerMetaType()
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
    qDBusRegisterMetaType<DeviceInfo>();
    qRegisterMetaType<FeatureInfo>("FeatureInfo");
    qDBusRegisterMetaType<FeatureInfo>();
}

QString GetDefaultDevice(const QString &userName)
{
//    QString configPath = QString("/home/%1/" UKUI_BIOMETRIC_CONFIG_PATH).arg(userName);
//    QSettings settings(configPath, QSettings::IniFormat);
//    qDebug() << "configure path: " << settings.fileName();
	
    QString configPath = QDir::homePath() + "/" + UKUI_BIOMETRIC_CONFIG_PATH; 
    QSettings settings(configPath, QSettings::IniFormat);

    QString defaultDevice = settings.value("DefaultDevice").toString();

    if(defaultDevice.isEmpty())
    {
        QString configPath = QString("/var/lib/lightdm-data/%1/" UKUI_BIOMETRIC_CONFIG_PATH).arg(userName);
	QSettings settings(configPath, QSettings::IniFormat);
	defaultDevice = settings.value("DefaultDevice").toString();
    }
     
    if(defaultDevice.isEmpty() && userName == "root"){
    	QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
        defaultDevice = sysSettings.value("rootUserDefaultDevice").toString();
    }

    if(defaultDevice.isEmpty())
    {
        QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
        defaultDevice = sysSettings.value("DefaultDevice").toString();
    }

    return QString(defaultDevice);
}

int GetLastDevice(const QString &userName)
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

void SetLastDevice(const QString &userName, int drvid)
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

static int getValueFromSettings(const QString &userName, const QString &key, int defaultValue = 3)
{
    //从家目录下的配置文件中获取
//    QString configPath = QString("/home/%1/" UKUI_BIOMETRIC_CONFIG_PATH).arg(userName);
    QString configPath = QDir::homePath() + "/" + UKUI_BIOMETRIC_CONFIG_PATH;
    QSettings settings(configPath, QSettings::IniFormat);
    QString valueStr = settings.value(key).toString();

    //如果没有获取到，则从系统配置文件中获取
    if(valueStr.isEmpty())
    {
        QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
        valueStr = sysSettings.value(key).toString();
    }

    bool ok;
    int value = valueStr.toInt(&ok);
    if( (value == 0 && !ok) || valueStr.isEmpty() )
    {
        value = defaultValue;
    }
    return value;
}

bool GetHiddenSwitchButton()
{
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    if(sysSettings.contains("HiddenSwitchButton"))
        return sysSettings.value("HiddenSwitchButton").toBool();
    else
        return false;
}

bool GetAuthEnable()
{
    bool isEnable = false;
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    if(sysSettings.contains("EnableAuth"))
        isEnable =  sysSettings.value("EnableAuth").toBool();
    else
        isEnable = false;

    if(isEnable && sysSettings.allKeys().contains("EnableAuthApp")){
        int isEnableApp = sysSettings.value("EnableAuthApp").toInt();
        isEnable = isEnableApp & 1;
    }

    return isEnable;
}

bool GetQRCodeEnable()
{
    bool isEnable = false;
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    sysSettings.beginGroup("Functions");
    if (sysSettings.allKeys().contains("EnableQRCode")) {
        isEnable = sysSettings.value("EnableQRCode").toBool();
    }
    sysSettings.endGroup();
    return isEnable;
}

int GetFailedTimes()
{
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    if(sysSettings.contains("MaxFailedTimes"))
        return sysSettings.value("MaxFailedTimes").toInt();
    else
        return 3;
}

bool GetUseFirstDevice()
{
    QSettings sysSettings(UKUI_BIOMETRIC_SYS_CONFIG_PATH, QSettings::IniFormat);
    return sysSettings.value("UseFirstDevice").toBool();
}

int GetMaxFailedAutoRetry(const QString &userName)
{
    return getValueFromSettings(userName, "MaxFailedAutoRetry");
}

int GetMaxTimeoutAutoRetry(const QString &userName)
{
    return getValueFromSettings(userName, "MaxTimeoutAutoRetry");
}
