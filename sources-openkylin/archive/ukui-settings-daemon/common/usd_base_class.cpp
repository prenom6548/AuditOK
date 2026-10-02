/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 * Copyright (C) 2012-2014 Daniel Vrátil <dvratil@redhat.com>
 * Copyright (C) 2012 Daniel Nicoletti <dantti12@gmail.com>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QX11Info>
#include <QGuiApplication>
#include "clib-syslog.h"
#include "usd_base_class.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QSettings>
#include <QDBusReply>
#include <QProcess>
#include <QDebug>
#include <QTime>
#include <QFile>
#include <QDir>
#include <QtMath>
#define STR_EQUAL 0

#define DBUS_SERVICE "org.freedesktop.UPower"
#define DBUS_OBJECT "/org/freedesktop/UPower"
#define DBUS_INTERFACE "org.freedesktop.DBus.Properties"
#define POWER_OFF_CONFIG_FILE "/sys/class/dmi/id/modalias"

static QString g_motify_poweroff = "";
#define UBC_UNSET 999
extern "C"{
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
}
#include "rfkillswitch.h"

UsdBaseClass::UsdBaseClass()
{
}

UsdBaseClass::~UsdBaseClass()
{

}

bool UsdBaseClass::isMasterSP1()
{
    return false;
}

bool UsdBaseClass::isUseXEventAsShutKey()
{
    return true;
}

bool UsdBaseClass::isTablet()
{
    return isTableWithDBus();
}

bool UsdBaseClass::isTableWithDBus()
{
    return false;
#if 0
    QDBusInterface statusFace(DBUS_STATUSMANAGER_NAME,
                                             DBUS_STATUSMANAGER_PATH,
                                             DBUS_STATUSMANAGER_INTERFACE,
                                             QDBusConnection::sessionBus());

    if (statusFace.isValid()) {
        QDBusReply<bool> replay = statusFace.call("getCurrentTabletMode");
        return replay.value();
    } else {
        USD_LOG(LOG_ERR, "m_statusManagerDbus error");
    }
    return false;
#endif
}

bool UsdBaseClass::is9X0()
{
#ifdef USD_9X0
     return true;
#endif
    return false;
}

bool UsdBaseClass::isWayland()
{
    static int isWaylandBackend = -1;
    char *pdata;

    if (isWaylandBackend != -1) {
        return isWaylandBackend;
    }

    pdata = getenv("XDG_SESSION_TYPE");
    USD_LOG_SHOW_PARAMS(pdata);

    if (pdata) {
        if (!strncmp(pdata,"x11",3)) {
            isWaylandBackend = 0;
            USD_LOG(LOG_DEBUG,"x11");
        } else {
            isWaylandBackend = 1;
            USD_LOG(LOG_DEBUG,"wayland");
        }
    }
    return isWaylandBackend;
}

bool UsdBaseClass::isWlcom()
{
    static int isWlcom = -1;

    if (isWlcom != -1) {
        return isWlcom;
    }
    char *data = getenv("XDG_SESSION_DESKTOP");
    if (data) {
        USD_LOG(LOG_DEBUG, "XDG_SESSION_DESKTOP == %s", data);
        if (!strncmp(data, "kylin-wlcom", 11)) {
            isWlcom = 1;
        } else {
            isWlcom = 0;
        }
    }
    return isWlcom;
}

bool UsdBaseClass::isXcb()
{
    if (QGuiApplication::platformName().startsWith(QLatin1String("xcb"))) {
        USD_LOG(LOG_DEBUG,"is xcb app");
        return true;
    }
    return false;
}

bool UsdBaseClass::isNotebook()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(DBUS_SERVICE,DBUS_OBJECT,DBUS_INTERFACE,"Get");
    msg<<DBUS_SERVICE<<"LidIsPresent";
    QDBusMessage res = QDBusConnection::systemBus().call(msg);
    if(res.type()==QDBusMessage::ReplyMessage)
    {
        QVariant v = res.arguments().at(0);
        QDBusVariant dv = qvariant_cast<QDBusVariant>(v);
        QVariant result = dv.variant();
        return result.toBool();
    }

    return false;
}

bool UsdBaseClass::readPowerOffConfig()
{
    QFile file;
    QString filePath = POWER_OFF_CONFIG_FILE;

    file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream pstream(&file);
    g_motify_poweroff = pstream.readAll();
    file.close();
    return true;
}

bool UsdBaseClass::isPowerOff()
{
    const QStringList devName ={"pnPF215T"};

    if(g_motify_poweroff.isEmpty())
        readPowerOffConfig();

    for(auto devNameTmp : devName)
    {
        if (g_motify_poweroff.contains(devNameTmp, Qt::CaseSensitive))
        {
            return true;
        }
    }
    return false;
}

bool UsdBaseClass::isJJW7200()
{
    static int ret = UBC_UNSET;
    char *pAck = NULL;
    char CmdAck[256] = "";
    FILE * pPipe;

    if (ret != UBC_UNSET) {
        return ret;
    }

    pPipe = popen("lspci | grep -i VGA |grep 7200","r");

    if (pPipe) {
        pAck = fgets(CmdAck, sizeof(CmdAck)-1, pPipe);
        if (strlen(CmdAck) > 3) {
            ret = true;
        } else{
            ret = false;
        }

        pclose(pPipe);
    } else {
        ret = false;
    }

    return ret;
}

int UsdBaseClass::getDPI()
{
    static int ret = 0;
    char *dpi = NULL;

    if (ret) {
        return ret;
    }
    if (UsdBaseClass::isXcb()) {
        dpi = XGetDefault(QX11Info::display(), "Xft", "dpi");
    }

    if (dpi) {
        QString qDpi = QString::fromLatin1(dpi);

        if (qDpi == "192") {
            ret = 192;
        } else {
            ret = 96;
        }
    } else {
        ret = 96;
    }
    return ret;
}


double UsdBaseClass::getScale(double scaling)
{
    double scale = 0.0;
    if (scaling <= 2.15)
        scale = getScoreScale(scaling);
    else if (scaling <= 3.15)
        scale = getScoreScale(scaling - 1) + 1;
    else if (scaling <= 4.15)
        scale = getScoreScale(scaling - 2) + 2;
    else if (scaling <= 5.15)
        scale = getScoreScale(scaling - 3) + 3;
    else if (scaling <= 6.15)
        scale = getScoreScale(scaling -4 ) + 4;
    else
        scale = 6;// 根据目前大屏及8K屏幕，最高考虑6倍缩放

    return scale/2;
}

double UsdBaseClass::getScaleWithSize(int heightmm, int widthmm, int height, int width)
{
    double inch = 0.0;
    double scale = 0.0;
    double screenArea  = height * width;
    inch = sqrt(heightmm * heightmm + widthmm * widthmm) / 25.4;

    if (inch <= 10.00) {
        scale = qSqrt(screenArea) / qSqrt(1024 * 576);
    }
    else if (10.00 < inch && inch <= 15.00) { // 10 < inch <= 15 : 1366x768
        scale = qSqrt(screenArea) / qSqrt(1366 * 768);
    }
    else if (15.00 < inch && inch <= 20.00) { // 15 < inch <= 20 : 1600x900
        scale = qSqrt(screenArea) / qSqrt(1600 * 900);
    }
    else if (20.00 < inch && inch <= 30.00) { // 20 < inch <= 30 : 1920x1080
        scale = qSqrt(screenArea) / qSqrt(1920 * 1080);
    }
    else if (30 < inch && inch<= 60) { // 30 < inch <= 60 :
        scale = qSqrt(screenArea) / qSqrt(1600 * 900);
    }
    else { // inch > 60
        scale = qSqrt(screenArea) / qSqrt(1280 * 720);
    }
    return getScale(scale);
}

double UsdBaseClass::getScoreScale(double scaling)
{
    double scale = 0.0;
    if (scaling <= 1.15)
        scale = 1;
    else if (scaling <= 1.4)
        scale = 1.25;
    else if (scaling <= 1.65)
        scale = 1.5;
    else if (scaling <= 1.9)
        scale = 1.75;
    else
        scale = 2;

    return scale;
}

void UsdBaseClass::writeUserConfigToLightDM(QString group, QString key, QVariant value, QString userName)
{
    QDir dirCheck;
    QString user = QDir::home().dirName();
    if (!userName.isEmpty()) {
        user = userName;
    }
//    qDebug()<<key<<":"<<value;
    QString usdDir = QString("/var/lib/lightdm-data/%1/usd").arg(user);
    QString configDir = QString("/var/lib/lightdm-data/%1/usd/config").arg(user);
    QString configFile = QString("/var/lib/lightdm-data/%1/usd/config/ukui-settings-daemon.settings").arg(user);

    if (!dirCheck.exists(usdDir)) {
        dirCheck.mkdir(usdDir);

        QFile file(usdDir);
        file.setPermissions(QFileDevice::Permission(0x7777));
        file.close();
    }

    if (!dirCheck.exists(configDir)) {
        dirCheck.mkdir(configDir);
    }
    QFile file(configDir);
    file.setPermissions(QFileDevice::Permission(0x7777));
    file.close();

    QSettings *usdSettings = new QSettings(configFile, QSettings::IniFormat);
    USD_LOG(LOG_DEBUG,"ready save %s writable:%d!",configFile.toLatin1().data(), usdSettings->isWritable());
    usdSettings->beginGroup(group);
    usdSettings->setValue(key, value);
    usdSettings->endGroup();
    usdSettings->sync();
    usdSettings->deleteLater();
    QFile::setPermissions(configFile,QFileDevice::Permission(0x6666));
}

QVariant UsdBaseClass::readUserConfigToLightDM(QString group, QString key, QString userName)
{
    QVariant ret;
    QString user = QDir::home().dirName();
    if (!userName.isEmpty()) {
        user = userName;
    }
    QString configFile = QString("/var/lib/lightdm-data/%1/usd/config/ukui-settings-daemon.settings").arg(user);

    QSettings *usdSettings = new QSettings(configFile, QSettings::IniFormat);
    usdSettings->beginGroup(group);
    ret = usdSettings->value(key);
    usdSettings->endGroup();
    usdSettings->sync();
    usdSettings->deleteLater();

    return ret;
}

bool UsdBaseClass::isWaylandWithKscreen()
{
    return isWayland();
}


/*
 *Normally, brightness are controlled by OS, but the following laptop hardware has control over them
*/
bool UsdBaseClass::brightnessControlByHardware(int &step)
{
    static int ret = -1;
    static int hardwareStep;
    //:rnLXKT-ZXE-N70: n70,n80z,n79
    QList<QString> m_appointBrightness = {":rnLXKT-ZXE-N70:"};

    if (ret != -1) {
        step = hardwareStep;
        return ret;
    }

    if (g_motify_poweroff.isEmpty()) {
        readPowerOffConfig();
    }

    Q_FOREACH (const QString &str, m_appointBrightness) {
        if (g_motify_poweroff.contains(str)) {
            ret = 1;
            hardwareStep = 5;
            step = hardwareStep;
            return ret;
        }
    }

    ret = false;
    return ret;
}

/*
 *Normally, the flight mode is controlled by hardware, but the following laptop hardware has surrendered control
*/
bool UsdBaseClass::flightModeControlByHardware(int &mode)
{
    static int ret = -1;
    QList<QString> m_flightControlByHardware = {":rnLXKT-ZXE-N70:"};

    if (ret != -1) {
        mode = RfkillSwitch::instance()->getCurrentFlightMode();
        return ret;
    }

    if (g_motify_poweroff.isEmpty()) {
        readPowerOffConfig();
    }

    Q_FOREACH (const QString &str, m_flightControlByHardware) {
        if (g_motify_poweroff.contains(str)) {
            ret = false;
            break;
        }
    }

    mode = RfkillSwitch::instance()->getCurrentFlightMode();
    ret = ret == -1 ? true : ret;
    return ret;
}


/*
 *Normally, touchpads are controlled by OS, but the following laptop hardware has control over them
*/
bool UsdBaseClass::touchpadControlByHardware(int &state)
{
    static int ret = -1;
    QList<QString> m_touchpadControlByHardware = {":rnLXKT-ZXE-N70:"};

    if (ret == false) {
        return ret;
    }

    if (g_motify_poweroff.isEmpty()) {
        readPowerOffConfig();
    }

    Q_FOREACH (const QString &str, m_touchpadControlByHardware) {
        if (g_motify_poweroff.contains(str)) {
            bool readOK;
            QVariant touchpadState;
            touchpadState = UsdBaseClass::readInfoFromFile(EC_TOUCHPADSTATE);

            state = touchpadState.toUInt(&readOK);
            if (readOK) {
                state = 0;
            } else {
                state = touchpadState.toUInt();
            }

            ret = true;
            return ret;
        }
    }

    ret = false;
    return ret;
}

/*
 *Normally, power mode are controlled by OS, but the following laptop hardware has control over them
*/
bool UsdBaseClass::powerModeControlByHardware(int &mode)
{
    static int ret = -1;
    QList<QString> m_appointPowerMode = {":rnLXKT-ZXE-N70:",":rn1DZX2SM-A351"};

    if (ret == false) {
        return ret;
    }

    if (g_motify_poweroff.isEmpty()) {
        readPowerOffConfig();
    }

    Q_FOREACH (const QString &str, m_appointPowerMode) {
        if (g_motify_poweroff.contains(str)) {
            QVariant hardMode;
            QFile file(PERFORMANCE_MODE);
            USD_LOG(LOG_DEBUG,"%s contanis %s",g_motify_poweroff.toLatin1().data(), str.toLatin1().data());
            if (file.exists()) {
                hardMode = UsdBaseClass::readInfoFromFile(PERFORMANCE_MODE);
                /*TODO
               此类机型只有两种模式
            */
                switch (hardMode.toInt()) {
                case 1:
                    mode = 0;//perfromace
                    break;
                case 2:
                    mode = 2;//eco
                    break;
                default:
                    break;
                }
                ret = true;
                return ret;
            } else {
                mode = -1;
                ret = true;
                return ret;
            }
        }
    }

    ret = false;
    return ret;
}

QVariant UsdBaseClass::readInfoFromFile(QString filePath)
{
     QString strData = "";
     QFile file(filePath);
     if (!file.exists()) {
         USD_LOG(LOG_ERR,"can't find :%s",filePath.toLatin1().data());
         return false;
     }
//         USD_LOG(LOG_ERR,"find :%s",filePath.toLatin1().data());
     if (file.open(QIODevice::ReadOnly)) {
         strData = QString::fromLocal8Bit(file.readAll());
         file.close();
     }

     return strData;
}

QString UsdBaseClass::readHashFromFile(QString filePath)
{
     QString strData = "";
     QFile file(filePath);

     if (!file.exists()) {
         USD_LOG(LOG_ERR,"can't find :%s",filePath.toLatin1().data());
         return "false";
     }

//     USD_LOG(LOG_ERR,"find :%s",filePath.toLatin1().data());
     if (file.open(QIODevice::ReadOnly)) {
         QDataStream in(&file); // 创建数据流与文件关联
         in.setVersion(QDataStream::Qt_5_0); // 设置数据流版本，确保兼容性

         // 读取文件内容到字节数组
         QByteArray readData = file.readAll();
         if (0 != readData.length()) {
             const quint8 *data = reinterpret_cast<const quint8 *>(readData.constData());
             QCryptographicHash hash(QCryptographicHash::Md5);
             hash.addData(reinterpret_cast<const char *>(data), readData.length());
             strData = QString::fromLatin1(hash.result().toHex());
         } else {
             QStringList paths = filePath.split("/");
             QString tempPath = paths[paths.count()-2];
             QStringList splitLine  = tempPath.split("-");

             QString screenName;
             for (int var = 1; var < splitLine.count(); ++var) {
                 screenName += splitLine[var];
                 if (splitLine.count()-1 != var) {
                     screenName += "-";
                 }
             }

             strData = screenName;
         }
         file.close();
     }

     return strData;
}

bool UsdBaseClass::isVirt()
{
    QString ret;
    QProcess process;
    process.start("systemd-detect-virt");
    process.waitForStarted();
    process.waitForFinished();
    ret = process.readAllStandardOutput();
    if (ret.contains("microsoft",Qt::CaseInsensitive) ||
            ret.contains("oracle",Qt::CaseInsensitive)  ||
            ret.contains("kvm",Qt::CaseInsensitive)) {
        return true;
    }
    USD_LOG_SHOW_PARAMS(ret.toLatin1().data());
    QFile ctFile("/usr/local/ctyun/clink/Mirror/Registry/Default");
    if (ctFile.exists()) {
        return true;
    }

    QFile fvendor("/sys/devices/virtual/dmi/id/chassis_vendor");
    QFile ftag("/sys/devices/virtual/dmi/id/chassis_asset_tag");
    QString strVendor;
    QString strTag;

    if (fvendor.exists()) {
        if (fvendor.open(QIODevice::ReadOnly)) {
            strVendor= fvendor.readAll();
            fvendor.close();
        }
    }
    USD_LOG_SHOW_PARAMS(strVendor.toLatin1().data());
    if (ftag.exists()) {
        if (ftag.open(QIODevice::ReadOnly)) {
            strTag = ftag.readAll();
            ftag.close();
        }
    }
    USD_LOG_SHOW_PARAMS(strVendor.toLatin1().data());
    if (strVendor.contains("Huawei Inc.",Qt::CaseInsensitive) || strTag.contains("HUAWEICLOUD",Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

int UsdBaseClass::getConnectOutputCount()
{
    int ret = 0;
    QDir dir(UKUI_DRM_PATH);
    if (!dir.exists()) {
        USD_LOG(LOG_ERR, "can't find drm..");
    }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subdir : dirs) {
        QString subDirPath = dir.absoluteFilePath(subdir);
        QString status;
        if (!(subDirPath.contains("card") && subDirPath.contains(("-")))) {
            continue;
        }

        status = readInfoFromFile(subDirPath + "/status").toString();
        if (status.contains("disconnected")) {
            continue;
        }
        ret++;
    }

    return ret;
}

QString UsdBaseClass::getConnectOutputHash()
{
    QString hash;
    QStringList edidList;
    QDir dir(UKUI_DRM_PATH);
    if (!dir.exists()) {
        USD_LOG(LOG_ERR, "can't find drm..");
    }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subdir : dirs) {
        QString subDirPath = dir.absoluteFilePath(subdir);
        QString status;
        QString edid;
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (!(subDirPath.contains("card") && subDirPath.contains(("-")))) {
            continue;
        }

        status = readInfoFromFile(subDirPath + "/status").toString();
        if (status.contains("disconnected")) {
            continue;
        }
        edid = readHashFromFile(subDirPath + "/edid");
        edidList << edid ;
    }
    std::sort(edidList.begin(), edidList.end());
    const auto hashHex = QCryptographicHash::hash(edidList.join(QString()).toLatin1(), QCryptographicHash::Md5);
    hash = QString::fromLatin1(hashHex.toHex());
//    USD_LOG_SHOW_PARAMS(hash.toLatin1().data());
    return hash;
}

QString UsdBaseClass::getOutputHash(QString name)
{
    QString hash = "";
    QMap<QString,QString> pnpMap = getPnpMap();
    QDir dir(UKUI_DRM_PATH);
    if (!dir.exists()) {
        USD_LOG(LOG_ERR, "can't find drm..");
    }

    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug()<<dirs;

    for (const QString &subdir : dirs) {
        QString subDirPath = dir.absoluteFilePath(subdir);
        QString status;

        if (!(subDirPath.contains("card") && subDirPath.contains(("-")))) {
            continue;
        }

        status = readInfoFromFile(subDirPath + "/status").toString();
        if (status.contains("disconnected")) {
            continue;
        }

        QString edidFile = subDirPath + "/edid";
        QFile file(edidFile);

        if (!file.exists()) {
            USD_LOG(LOG_ERR,"can't find :%s",edidFile.toLatin1().data());
            return "false";
        }

        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file); // 创建数据流与文件关联
            in.setVersion(QDataStream::Qt_5_0); // 设置数据流版本，确保兼容性

            // 读取文件内容到字节数组
            QByteArray readData = file.readAll();
            EdidParser ep(readData.constData(), readData.length());
            USD_LOG_SHOW_PARAMS(ep.m_model.toLatin1().data());
            USD_LOG_SHOW_PARAMS(ep.m_pnpId.toLatin1().data());
            USD_LOG_SHOW_PARAMS(ep.m_eisaId.toLatin1().data());
            USD_LOG_SHOW_PARAMS(ep.m_monitorName.toLatin1().data());
            USD_LOG_SHOW_PARAMS(pnpMap[ep.m_pnpId].toLatin1().data());
            QString fullPnpId = pnpMap[ep.m_pnpId];
            QString nameByPnp;

            if (fullPnpId.isEmpty()) {
                nameByPnp = QString("%1 %2").arg(ep.m_pnpId).arg(ep.m_model);
            } else {
                nameByPnp = QString("%1 %2").arg(pnpMap[ep.m_pnpId]).arg(ep.m_model);
            }

            USD_LOG_SHOW_PARAMS(name.toLatin1().data());
            USD_LOG_SHOW_PARAMS(nameByPnp.toLatin1().data());
            if (name != nameByPnp) {
                continue;
            }

            hash = readHashFromFile(edidFile);
            USD_LOG_SHOW_PARAMS(hash.toLatin1().data());
            break;
        }
    }

    return hash;
}

QString UsdBaseClass::getConnectOutputHashWithPort()
{
    QString ret;
    QStringList edidList;
    QDir dir(UKUI_DRM_PATH);
    if (!dir.exists()) {
        USD_LOG(LOG_ERR, "can't find drm..");
    }
    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug()<<dirs;

    for (const QString &subdir : dirs) {
        QString subDirPath = dir.absoluteFilePath(subdir);
        QString status;
        if (!(subDirPath.contains("card") && subDirPath.contains(("-")))) {
            continue;
        }

        status = readInfoFromFile(subDirPath + "/status").toString();
        if (status.contains("disconnected")) {
            continue;
        }
        status = subdir;
//        edidList << status.replace(0, 4, "");
          edidList << status.replace("card-", "");
    }
    std::sort(edidList.begin(), edidList.end());
    const auto hashHex = QCryptographicHash::hash(edidList.join(QString()).toLatin1(), QCryptographicHash::Md5);
    ret = QString::fromLatin1(hashHex.toHex());
    return ret;
}


QString UsdBaseClass::parseVender(const QByteArray &rawData)
{
    EdidParser ep(rawData.constData(), rawData.length());
    QString ret = QString("%1 %2").arg(ep.m_pnpId).arg(ep.m_model);
    return ret;
}

QString UsdBaseClass::getOutputVender(const QString outputTypeName)
{
    QString ret = "";
    QDir dir(UKUI_DRM_PATH);
    if (!dir.exists()) {
        USD_LOG(LOG_ERR, "can't find drm..");
    }
    QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
//    USD_LOG(LOG_DEBUG,"ready check:%s",outputTypeName.toLatin1().data());
    for (const QString &subdir : dirs) {
        QString subDirPath = dir.absoluteFilePath(subdir);
        QString status;

        if (!subdir.contains(outputTypeName)) {
            continue;
        }

        if (!(subDirPath.contains("card") && subDirPath.contains(("-")))) {
            continue;
        }

        status = readInfoFromFile(subDirPath + "/status").toString();
        if (status.contains("disconnected")) {
            continue;
        }

        QString edidFile = subDirPath + "/edid";
        QFile file(edidFile);

        if (!file.exists()) {
            USD_LOG(LOG_ERR,"can't find :%s",edidFile.toLatin1().data());
            return "false";
        }
//        USD_LOG(LOG_ERR,"find :%s",edidFile.toLatin1().data());
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file); // 创建数据流与文件关联
            in.setVersion(QDataStream::Qt_5_0); // 设置数据流版本，确保兼容性

            // 读取文件内容到字节数组
            QByteArray readData = file.readAll();
            ret = parseVender(readData);
        }

        break;
    }
    return ret;
}


QMap<QString, QString> UsdBaseClass::getPnpMap()
{
    static QMap<QString,QString>m_pnpMap;

    if (m_pnpMap.count() == 0) {
        QFile pnpIds(QStringLiteral("/usr/share/hwdata/pnp.ids"));
        if (pnpIds.open(QIODevice::ReadOnly)) {
            const QStringList pnpList = QString::fromLatin1(pnpIds.readAll()).split("\n");

            Q_FOREACH(const QString str, pnpList) {
                const QStringList pnp = str.split(QLatin1Char('\t'));
                m_pnpMap.insert(pnp[0],pnp[1]);
            }
        }
    }
    return m_pnpMap;
}

void UsdBaseClass::printTimeWithMsg(QString msg, QString fun, int line)
{
    QTime time = QTime::currentTime();

    SYS_LOG(LOG_DEBUG,"%02d:%02d:%02d:%03d %s %d--->%s",time.hour(),
            time.minute(),
            time.second(),
            time.msec(),
            fun.toLatin1().data(),
            line,
            msg.toLatin1().data());
}

QString UsdBaseClass::readAllText(QString filePath)
{
    QString text = "";
    QFile file(filePath);
    if (!file.exists()) {
        return text;
    }

    if (file.open(QIODevice::ReadOnly)) {
        text = file.readAll();
        file.close();
    }

    return text;
}

bool UsdBaseClass::isSupportGammaWithPciID(QString ueventInfo)
{
    constexpr auto DRIVER_PREFIX = "DRIVER=";
    constexpr auto PCI_ID_PREFIX = "PCI_ID=";
    QStringList keyList = ueventInfo.split("\n");
    static const QMap<QString, QStringList> blackMap = {
        {DRIVER_PREFIX, {"loongson-drm","gsgpu"}},/*null、229739*/
        {PCI_ID_PREFIX, {"0709:0001"}}
    };

    //没有驱动的情况下drm下面是空的，没有uevnet文件，所以info为空也是不支持。
    if (ueventInfo.isEmpty()) {
        return false;
    }

    Q_FOREACH(const QString key, keyList) {
        if (key.isEmpty()) {
            break;
        }

        for (auto it = blackMap.constBegin(); it != blackMap.constEnd(); ++it) {
            if (!key.startsWith(it.key(), Qt::CaseInsensitive)) {
                continue;
            }

            const QString value = key.section('=', 1).trimmed();
            if (it.value().contains(value)) {
                return false;
            }
        }
    }
    return true;
}

bool UsdBaseClass::isSupportGammaWithRender()
{
    QDBusInterface statusFace(ORG_UKUI_DBUS_WLCOM_NAME,
                                             ORG_UKUI_DBUS_WLCOM_RENDERER_PATH,
                                             ORG_UKUI_DBUS_WLCOM_RENDERER_INTERFACE,
                                             QDBusConnection::sessionBus());

    if (statusFace.isValid()) {
        QDBusReply<bool> replay = statusFace.call(ORG_UKUI_DBUS_WLCOM_RENDERER_ISHADRWARE);
        if (replay.isValid()) {
            SYS_LOG(LOG_DEBUG, "isSupportGammaWithRender %d", replay.value());
            return replay.value();
        } else {
            SYS_LOG(LOG_WARNING, "call isSupportGammaWithRender error");
        }

    } else {
        SYS_LOG(LOG_WARNING, "isSupportGammaWithRender error");
    }
    return true;
}

double UsdBaseClass::getNativeScaleFactor(double widthMM, double heightMM, int resolutionWidth, int resolutionHeight)
{
    double nativeScaleFactor = 0.0;
    double screenSize = 0.0;
    double screenSizeInch = 0.0;
    double deviceType = 0.0;

    screenSize = qSqrt(qPow(widthMM, 2) + qPow(heightMM, 2));
    screenSizeInch = screenSize  / 2.54;
    nativeScaleFactor = qSqrt(qPow(resolutionWidth, 2) + qPow(resolutionHeight, 2)) / screenSizeInch;

    if (screenSizeInch >= 20) {
        deviceType = 28; //外接显示器
    } else if (screenSizeInch >= 12) {
        deviceType = 24.5; //笔记本内屏
    } else {
        deviceType = 20; //平板类
    }

    nativeScaleFactor = nativeScaleFactor * deviceType / (96 * 28);

    return nativeScaleFactor;
}

double UsdBaseClass::getPreferredScaleFactor(double nativeDpi, int resolutionWidth, int resolutionHeight)
{
    double scaleFactor = 1;
    if (nativeDpi < 1.2) {
        scaleFactor = 1.0;
    } else if (nativeDpi < 1.43) {
        scaleFactor = 1.25;
    } else if (nativeDpi < 1.78) {
        if (resolutionWidth <= 1920 || resolutionHeight <=1200) {
            scaleFactor = 1.25;
        } else {
            scaleFactor = 1.5;
        }
    } else if (nativeDpi < 2.32) {
        scaleFactor = 2.0;
    } else {
        scaleFactor = 2.0;
    }
    return scaleFactor;
}
/*
DRIVER=loongson-drm
PCI_CLASS=30000
PCI_ID=0014:7A06
PCI_SUBSYS_ID=0000:0000
PCI_SLOT_NAME=0000:00:06.1
MODALIAS=pci:v00000014d00007A06sv00000000sd00000000bc03sc00i00
*/
bool UsdBaseClass::isSupportGamma()
{
    static bool ret = isSupportGammaWithRender();

    return ret;
}

bool UsdBaseClass::isSupportGammaWithLspci()
{
    static int ret = -1;
    char Cmd[512] = {0};
    char *pAck = NULL;
    char CmdAck[120];
    FILE * pPipe;
    QString strAck;
    if (ret >= 0) {
        return ret;
    }

    sprintf(Cmd, "lspci |grep VGA");
    pPipe = popen(Cmd,"r");

    if (pPipe) {
        pAck = fgets(CmdAck,sizeof(CmdAck),pPipe);
        pclose(pPipe);
    }

    strAck = QString::fromLatin1(CmdAck);
    USD_LOG_SHOW_PARAMS(strAck.toLatin1().data());
    ret = strAck.contains("loongson", Qt::CaseInsensitive);
    return !ret;
}


bool UsdBaseClass::writeGlobalConfig(QString path, QString fileName, QByteArray info)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());

    QDBusReply<bool> reply = globalManager.call("writeGlobalConfig", path, fileName, info);

    if (reply.isValid()) {
        return reply.value();
    }

    SYS_LOG(LOG_WARNING,"writeGlobalConfig dbus interface failed .%s", fileName.toLatin1().data());
    return false;
}

QByteArray UsdBaseClass::readGlobalConfig(QString path)
{
    QFile globalFile;
    QString filePath = QString("%1/%2").arg(GLOBAL_CONFIG_PATH).arg(path);
    if (filePath.contains("../")) {
        SYS_LOG(LOG_DEBUG,"%s is error path", path.toLatin1().data());
        return QByteArray();
    }
    globalFile.setFileName(filePath);

    if (!QFile::exists(filePath)) {
        SYS_LOG(LOG_DEBUG,"read %s error",filePath.toLatin1().data());
        return QByteArray();
    }

    if (!globalFile.open(QIODevice::ReadOnly)) {
        SYS_LOG(LOG_DEBUG,"open %s error",filePath.toLatin1().data());
        return QByteArray();
    }

    return globalFile.readAll();
}

QByteArray UsdBaseClass::readGlobalConfigWithDbus(QString path)
{
    QByteArray info;

    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());

    QDBusReply<QByteArray> reply = globalManager.call("readGlobalConfig", path);

    if (reply.isValid()) {
        return reply.value();
    }

    SYS_LOG(LOG_WARNING,"writeGlobalConfig dbus interface failed .%s", path.toLatin1().data());

    return info;
}

bool UsdBaseClass::globalConfigExists(QString path)
{
    QString filePath = QString("%1/%2").arg(GLOBAL_CONFIG_PATH).arg(path);
    if (filePath.contains("../")) {
        SYS_LOG(LOG_DEBUG,"%s is error path", filePath.toLatin1().data());
        return false;
    }
    return QFile::exists(filePath);
}

bool UsdBaseClass::globalConfigExistsWithDbus(QString path)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/globalconfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());

    QDBusReply<bool> reply = globalManager.call("globalConfigExists", path);

    if (reply.isValid()) {
        return reply.value();
    }

    SYS_LOG(LOG_WARNING,"globalConfigExists dbus interface failed .%s", path.toLatin1().data());
    return false;
}

double UsdBaseClass::getDisplayScale()
{
    static double ret = 0;
    if (ret != 0) {
        return ret;
    }

    if (!UsdBaseClass::isWayland()) {
        ret = getDPI() / 96.0;
    } else {
        return 1.0;
    }

    return ret;
}

/**
 * @brief UsdBaseClass::getPreferredScale
 * @param widthMM
 * @param heightMM
 * @param resolutionWidth
 * @param resolutionHeight
 * @param scale //显示器的最佳缩放率，如果多屏是需要根据当前dpi再次组合计算，但是没有dpi时此值必须为1。供时save-param使用
 * @return
 */
double UsdBaseClass::getPreferredScale(double widthMM, double heightMM, int resolutionWidth, int resolutionHeight, double scale)
{
    double nativeDpi;
    widthMM /= 10;
    heightMM /= 10;
    USD_LOG_SHOW_PARAM2F(widthMM, heightMM);
    if (widthMM == 0 || heightMM == 0) {
        USD_LOG(LOG_DEBUG,"find bug, ");

        if(resolutionWidth >= 2160) {//华为产线无法获取尺寸，客户需求时150%。使用150%缩放。
            nativeDpi = 1.5;
        } else if (resolutionWidth >= 2000) {
            nativeDpi = 1.25;
        } else {
            nativeDpi = 1;
        }
    } else {
        nativeDpi = getNativeScaleFactor(widthMM, heightMM, resolutionWidth, resolutionHeight);
        nativeDpi = getPreferredScaleFactor(nativeDpi, resolutionWidth, resolutionWidth);
    }

    if (scale == 0) {
        USD_LOG_SHOW_PARAMF(UsdBaseClass::getDisplayScale());
        USD_LOG_SHOW_PARAMF((nativeDpi / UsdBaseClass::getDisplayScale()));
        nativeDpi /= UsdBaseClass::getDisplayScale();
    } else {
        USD_LOG_SHOW_PARAMF(scale);
        USD_LOG_SHOW_PARAMF(nativeDpi / scale);
        nativeDpi /= scale;
    }

    if (nativeDpi < 1) {
        nativeDpi = 1;
    }

    return nativeDpi;
}

bool UsdBaseClass::inTrialMode()
{
    static int ret = -1;

    if (ret != -1) {
        return ret;
    }
    ret = false;
    QString str = "";
    QStringList symbList ;
    QFile file("/proc/cmdline");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        str = QString::fromLocal8Bit(data);
        symbList = str.split("\r\n");
    }

    USD_LOG(LOG_DEBUG,"cmdline:%s",str.toLatin1().data());
    file.close();

    if (str.contains("boot=casper")) {
        ret = true;
    }

    if (getuid() == 999) {
        ret = true;
    }

    return ret;
}


QString UsdBaseClass::getFileHash(QString filePath)
{
    QByteArray fileContent;
    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return "";
        }
        fileContent = file.readAll();
        file.close();
    }
    QByteArray md5QBA = QCryptographicHash::hash(fileContent, QCryptographicHash::Md5);
    QString mdtStr = md5QBA.toHex();
    return mdtStr;
}

QString UsdBaseClass::updateSecurityConfig(QString filePath)
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/securityConfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());
    QDBusMessage qMessage = globalManager.call("updateConfig", filePath, QDir::home().dirName());
    return qMessage.errorName();
}

QString UsdBaseClass::clearSecurityConfig()
{
    QDBusInterface globalManager(SETTINGS_DAEMON_SYSTEMDBUS_NAME, \
                                 "/securityConfig", \
                                 SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE, \
                                 QDBusConnection::systemBus());
    QDBusMessage qMessage = globalManager.call("clearConfig", QDir::home().dirName());
    return qMessage.errorName();
}


const QStringList UsdBaseClass::getSecurityGsettingsNode()
{
    const QStringList gsettingsNode = {"config",
                            "org.ukui.peony.settings",
                            "org.ukui.menu.settings",
                            "org.ukui.panel.calendar",
                            "org.ukui.SettingsDaemon.plugins.customized"};
    return gsettingsNode;
}


bool UsdBaseClass::peekDir(QString dirPath, QFileDevice::Permission permission)
{
    QDir dirCheck;
    if (!dirCheck.exists(dirPath)) {
        dirCheck.mkpath(dirPath);
    }

    QFile file(dirPath);
    file.setPermissions(permission);
    file.close();

    return true;
}

bool UsdBaseClass::setAttributesToAppendOnly(QString path)
{
    QString cmd = QString::fromLatin1("chattr +a %1").arg(path);
    QProcess::startDetached(cmd);
    USD_LOG(LOG_DEBUG,"%s",cmd.toLatin1().data());

    return true;
}

bool UsdBaseClass::unsetAttributesToAppendOnly(QString path)
{
    QString cmd = QString::fromLatin1("chattr -a %1").arg(path);
    QProcess::startDetached(cmd);
    USD_LOG(LOG_DEBUG,"%s",cmd.toLatin1().data());

    return true;
}

const QStringList UsdBaseClass::get41NodeSchema()
{
//    const QStringList m_oldSchema = {
//        "org.ukui.peony.settings",
//        "org.ukui.panel.showdesktop",
//        "org.ukui.menu.settings",
//        "org.ukui.SettingsDaemon.plugins.customized"};

        const QStringList m_oldSchema = {
            "config"};

        return m_oldSchema;
}

bool UsdBaseClass::upmSupportAdjustBrightness()
{
    static int ret = -1;
    if (ret > -1) return ret;
    ret = 0;
    QDir brightnessNodeDir(BRIGHTNESS_NODE_PATH);
    QStringList nodeDirs = brightnessNodeDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    Q_FOREACH (auto &dir, nodeDirs) {
        QString node = brightnessNodeDir.absoluteFilePath(dir) + "/brightness";
        if (QFile::exists(node)) {
            SYS_LOG(LOG_DEBUG,"%s exist", node.toLatin1().data());
            ret = 1;
            break;
        } else {
            SYS_LOG(LOG_DEBUG,"%s is't exist", node.toLatin1().data());
        }
    }
    return ret;
}
