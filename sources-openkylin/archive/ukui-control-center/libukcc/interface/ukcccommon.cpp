/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "ukcccommon.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <QPainter>
#include <QPainterPath>
#include <QtDBus/QDBusMessage>

#include <cstring>  // strdup / free

using namespace ukcc;

UkccCommon::UkccCommon()
{
}

UkccCommon::~UkccCommon()
{

}

void UkccCommon::centerToScreen(QWidget* widget) {
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

QVariantMap UkccCommon::getModuleHideStatus() {
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected()) {
        qWarning() << "getModuleHideStatus: session bus is not connected";
        return {};
    }

    // 不用 QDBusInterface：构造时会同步 introspect，session 未就绪时主线程会卡死在 QWaitCondition
    QDBusMessage message = QDBusMessage::createMethodCall(
                QStringLiteral("org.ukui.ukcc.session"),
                QStringLiteral("/"),
                QStringLiteral("org.ukui.ukcc.session.interface"),
                QStringLiteral("getModuleHideStatus"));
    QDBusReply<QVariantMap> obj_reply(connection.call(message, QDBus::Block, 3000));
    if (!obj_reply.isValid()) {
        qWarning() << "execute dbus method getModuleHideStatus failed" << obj_reply.error();
        return {};
    }
    return obj_reply.value();
}

QString UkccCommon::getCpuInfo() {
    QFile file("/proc/cpuinfo");

    if (file.open(QIODevice::ReadOnly)) {
        QString buffer = file.readAll();
        QStringList modelLine = buffer.split('\n').filter(QRegularExpression("^model name"));
        QStringList modelLineWayland = buffer.split('\n').filter(QRegularExpression("^Hardware"));
        QStringList lines = buffer.split('\n');

        if (modelLine.isEmpty()) {
            if (modelLineWayland.isEmpty()) {
                return "Unknown";
            }
            modelLine = modelLineWayland;
        }


        int count = lines.filter(QRegularExpression("^processor")).count();

        QString result;
        QStringList modelArr = modelLine.first().split(':');
        for (int idx = 1; idx < modelArr.size(); ++idx) {
            if (idx == 1) {
                result.append(modelArr.at(idx));
            } else {
                result.append(":" + modelArr.at(idx));
            }
        }
        result = result.trimmed();

        return result;
    }

    return QString();
}


QString UkccCommon::getCpuArchitecture()
{
    QString cpuArchitecture;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("lscpu");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    QString cpuinfo = QString(ba.data());
    QStringList cpuinfo_list = cpuinfo.split("\n");
    for (int i = 0; i < cpuinfo_list.count(); i++) {
        QString mstring = cpuinfo_list.at(i);
        if (mstring.contains("Architecture")) {
            // 去除空格
            mstring = mstring.remove(QRegExp("\\s"));
            QStringList list = mstring.split(":");
            cpuArchitecture = list.at(1);
            break;
        }
    }
    return cpuArchitecture;
}

bool UkccCommon::isExistEffect() {
    QString filename = QDir::homePath() + "/.config/ukui-kwinrc";
    QSettings kwinSettings(filename, QSettings::IniFormat);

    QStringList keys = kwinSettings.childGroups();

    kwinSettings.beginGroup("Plugins");
    bool kwin = kwinSettings.value("blurEnabled", kwin).toBool();

    if (!kwinSettings.childKeys().contains("blurEnabled")) {
        kwin = true;
    }
    kwinSettings.endGroup();

    QFileInfo dir(filename);
    if (!dir.isFile()) {
        return true;
    }

    if (keys.contains("Compositing")) {
        kwinSettings.beginGroup("Compositing");
        QString xder;
        bool kwinOG = false;
        bool kwinEN = true;
        xder = kwinSettings.value("Backend", xder).toString();
        kwinOG = kwinSettings.value("OpenGLIsUnsafe", kwinOG).toBool();
        kwinEN = kwinSettings.value("Enabled", kwinEN).toBool();
        if ("XRender" == xder || kwinOG || !kwinEN) {
            return false;
        } else {
            return true;
        }
        kwinSettings.endGroup();
    }
    return true;
}

void UkccCommon::setKwinMouseSize(int size) {

    QString filename = QDir::homePath() + "/.config/kcminputrc";
    QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);

    mouseSettings->beginGroup("Mouse");
    mouseSettings->setValue("cursorSize", size);
    mouseSettings->endGroup();

    delete mouseSettings;
    mouseSettings = nullptr;

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(5);
    args.append(0);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

bool UkccCommon::isWayland() {
    QString sessionType = getenv("XDG_SESSION_TYPE");

    if (!sessionType.compare("wayland", Qt::CaseSensitive)) {
       return true;
    } else {
        return false;
    }
}

bool UkccCommon::isOpenkylin()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool UkccCommon::isOpenkylinNile()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    QString version = QString(QLatin1String(kdk_system_get_version(0)));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0
            && version.contains("nile", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UkccCommon::isOpenkylinYangtze()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    QString version = QString(QLatin1String(kdk_system_get_version(0)));
    if (systemName.compare("openkylin", Qt::CaseInsensitive) == 0
            && version.contains("yangtze", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UkccCommon::isCommunity()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("22.04", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

QRect UkccCommon::sizeOnCursor()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    return desk_rect;
}

bool UkccCommon::isTablet()
{
    QString projectName = QString(QLatin1String(kdk_system_get_projectSubName()));

    if (projectName.compare("mavis", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

bool UkccCommon::isExitBattery()
{
    /* 默认机器没有电池 */
    bool hasBat = false;
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus());
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");

    if (briginfo.value().toBool()) {
        hasBat = true ;
    }
    delete brightnessInterface;

    return hasBat;
}


QString UkccCommon::getHostName()
{
    QString hostname;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("hostname");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    hostname = ba.data();

    hostname.replace(QString("\n"),QString(""));
    return hostname;
}

bool UkccCommon::isZJY()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("PROJECT_CODENAME").toString();

    if (versionID.compare("v10sp1-zyj", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

bool UkccCommon::isV11()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("v11", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

static char* toHeap(const QString& s)
{
    QByteArray ba = s.toUtf8();
    return strdup(ba.constData());  // Linux/Unix
}

bool UkccCommon::buriedSettings(QString pluginName, QString settingsName,
                                QString action, QString value)
{
    KTrackData *trackData = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);

    kdk_dia_upload_default(trackData, "openfile", "mainwindow");

    trackData->eventCode  = strdup("");
    trackData->pageName   = toHeap(pluginName);

    // 如果 kdk_dia_data_free 会释放 customProperty 数组本身，这里也要用 malloc/new[]
    KCustomProperty* pt = (KCustomProperty*)malloc(sizeof(KCustomProperty) * 3);
    pt[0].key   = strdup("messageType");
    pt[0].value = toHeap(action);
    pt[1].key   = strdup("settingsName");
    pt[1].value = toHeap(settingsName);
    pt[2].key   = strdup("value");
    pt[2].value = toHeap(value);

    trackData->customProperty = pt;
    trackData->customPropertyLength = 3;

    kdk_dia_upload_default(trackData, trackData->eventCode, trackData->pageName);

    kdk_dia_data_free(trackData);  // 现在可以安全地释放所有堆内存

    return true;
}
QString UkccCommon::boolToString(bool b)
{
    if (b) {
        return QString("true");
    } else {
        return QString("false");
    }
}

QString UkccCommon::getUkccVersion()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = "none";

    pp = popen("dpkg -l  ukui-control-center | grep  ukui-control-center", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    line = NULL;
    pclose(pp);
    return version;
}

QString UkccCommon::getProductName()
{
    QDBusInterface ifc("com.control.center.qt.systemdbus",
                       "/",
                       "com.control.center.interface",
                       QDBusConnection::systemBus());
    QDBusReply<QString> res = ifc.call("getDmiDecodeRes", "system-product-name");
    return res.value().trimmed();
}

/*
* 判断用户是否为域用户
* 和/etc/passwd文件中用户做对比
* 1：域用户，0：非域用户
*/
bool UkccCommon::isDomainUser(const char* username)
{
    FILE *fp = fopen("/etc/passwd", "r");
    if (fp == NULL) {
        return true;
    }

    char buf[1024];
    while(!feof(fp)) {
        if (fgets(buf,sizeof (buf),fp) == NULL) {
           break;
        }
        char *colon = strchr(buf, ':');
        if (colon != NULL) {
            size_t len = (size_t)(colon - buf);
            if (strncmp(buf, username, len) == 0) {
                fclose(fp);
                return false;
            }
        }
    }

    fclose(fp);
    return true;
}

QPixmap UkccCommon::makeRoundLogo(QString logo, int wsize, int hsize, int radius, qreal dpi)
{
    QPixmap rectPixmap;
    QPixmap iconcop = QPixmap(logo);
    if (dpi > 1.0) {
        wsize = wsize * dpi;
        hsize = hsize * dpi;
    }

    if (iconcop.width() > iconcop.height()) {
        QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(hsize, Qt::SmoothTransformation);
    } else {
        QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(wsize, Qt::SmoothTransformation);
    }

    if (rectPixmap.isNull()) {
        return QPixmap();
    }
    QPixmap pixmapa(rectPixmap);
    QPixmap pixmap(radius * 2 * dpi, radius * 2 * dpi);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius * 2 * dpi, radius * 2 * dpi);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius * 2 * dpi, radius * 2 * dpi, pixmapa);
    pixmap.setDevicePixelRatio(dpi);
    return pixmap;
}

bool UkccCommon::isCloudVirtualDesktop()
{
    QString filename = QDir::homePath() + "/.config/activation_conf.ini";
    QSettings cloudSettings(filename, QSettings::NativeFormat);
    cloudSettings.beginGroup("MODE_Activation");

    bool isCloudVirtual = cloudSettings.value("cloud_virtual_desktop", "").toBool();

    cloudSettings.endGroup();

    return isCloudVirtual;

}
bool UkccCommon::isServerVersion()
{
    QString systemName = QString(QLatin1String(kdk_system_get_systemName()));
    if (systemName.contains("server", Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool UkccCommon::isExistMfa()
{
    QProcess *process = new QProcess;
    process->start("bash", QStringList() << "-c" << "dpkg -l | grep kylin-mfa");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    delete process;
    QString mOutput = QString(ba.data());

    // 判断多因子服务是否存在
    if (mOutput.contains("kylin-mfa")) {
        return true;
    }

    return false;
}

bool UkccCommon::isExistOpensshServer()
{
    QProcess *process = new QProcess;
    process->start("bash", QStringList() << "-c" << "dpkg -l | grep openssh-server");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();
    delete process;
    QString mOutput = QString(ba.data());

    // 判断多因子服务是否存在
    if (mOutput.contains("openssh-server")) {
        return true;
    }

    return false;
}

bool UkccCommon::isSupportTailLight()
{
    // devel/yhkylin/v11 分支默认开启
    return true;
}

bool UkccCommon::checkPamPwHistory()
{
    QFile file("/etc/pam.d/common-password");

    // 检查文件是否能打开
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << file.errorString();
        return false;
    }

    QTextStream in(&file);
    bool foundEnabled = false;

    // 正则表达式匹配:
    // ^\s*  - 行首可能有空白字符
    // (?!#) - 否定前瞻，确保不是注释行（不以#开头）
    // .*    - 任意字符
    // pam_pwhistory\.so - 目标字符串
    QRegularExpression regex(R"(^\s*(?!#).*pam_pwhistory\.so)");

    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        QRegularExpressionMatch match = regex.match(line);
        if (match.hasMatch()) {
            qDebug() << QString("Found enable setting on line %1 :").arg(lineNumber) << line.trimmed();
            foundEnabled = true;
        }
    }

    file.close();
    return foundEnabled;
}

bool UkccCommon::isProcessRunning(const QString &procName)
{
    QProcess process;

    /* 方法1: 使用 pgrep 命令查找进程 */
    process.start("pgrep", QStringList() << "-f" << procName);
    process.waitForFinished();

    if (process.exitCode() == 0) {
        QString output = process.readAllStandardOutput();
        return !output.trimmed().isEmpty();
    }

    /* 方法2: 如果 pgrep 不可用，使用 ps 命令作为备选 */
    process.start("ps", QStringList() << "aux");
    process.waitForFinished();

    QString output = process.readAllStandardOutput();
    return output.contains(procName);
}
