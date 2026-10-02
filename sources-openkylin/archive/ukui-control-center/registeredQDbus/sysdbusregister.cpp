/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "sysdbusregister.h"

#include <QDebug>
#include <QSharedPointer>
#include <QRegExp>
#include <QProcess>
#include <stdlib.h>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QStringList>

#include <QDBusInterface>
#include <QDBusReply>
#include <QCryptographicHash>
#include <polkit-qt5-1/polkitqt1-authority.h>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/kysdk-system/libkypackages.h>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#include <QtConcurrent/QtConcurrent>
#include <QDBusMessage>
#include "freedesktophelper.h"

namespace {
QMutex g_packageInstallCallbackMutex;
QPointer<SysdbusRegister> g_packageInstallCallbackReceiver;
QString g_packageInstallCallbackRequestId;
QString g_packageInstallCallbackPackageName;
QMutex g_packageUninstallCallbackMutex;
QPointer<SysdbusRegister> g_packageUninstallCallbackReceiver;
QString g_packageUninstallCallbackRequestId;
QString g_packageUninstallCallbackPackageName;

struct PackageNameInfo
{
    QString name;
    QString version;
};

struct SystemPackageInstallResult
{
    QString requestId;
    QString packageName;
    QString errorText;
    bool success = false;
};

// 确保包管理命令可被 SDK 找到
void ensurePackageManagerEnvironment()
{
    static const char * const defaultPathEntries[] = {
        "/usr/local/sbin",
        "/usr/local/bin",
        "/usr/sbin",
        "/usr/bin",
        "/sbin",
        "/bin",
    };

    QStringList pathEntries = QString::fromLocal8Bit(qgetenv("PATH"))
            .split(QLatin1Char(':'), Qt::SkipEmptyParts);
    bool changed = pathEntries.isEmpty();

    for (const char *entry : defaultPathEntries) {
        const QString pathEntry = QString::fromLatin1(entry);
        if (!pathEntries.contains(pathEntry)) {
            pathEntries.append(pathEntry);
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    const QByteArray pathBytes = pathEntries.join(QLatin1Char(':')).toLocal8Bit();
    qputenv("PATH", pathBytes);
    qInfo() << "set package manager PATH:" << pathBytes;
}

// 拆分软件包名和版本
PackageNameInfo packageNameInfo(const QString &packageName)
{
    const QString normalizedPackageName = packageName.trimmed();
    const int versionSeparator = normalizedPackageName.indexOf(QLatin1Char('='));
    if (versionSeparator > 0) {
        return {normalizedPackageName.left(versionSeparator),
                normalizedPackageName.mid(versionSeparator + 1)};
    }
    return {normalizedPackageName, QString()};
}

// 校验软件包名格式
bool isValidPackageName(const QString &packageName)
{
    static const QRegularExpression packageNameRegex(
        QStringLiteral("^[A-Za-z0-9][A-Za-z0-9+._:-]*([=][A-Za-z0-9][A-Za-z0-9+.:~_-]*)?$"));
    return packageNameRegex.match(packageName.trimmed()).hasMatch();
}

// 规范化并去重软件包名列表
QStringList normalizedPackageNames(const QStringList &packageNames, bool *valid)
{
    if (valid) {
        *valid = true;
    }

    QStringList normalizedNames;
    for (const QString &packageName : packageNames) {
        const QString normalizedPackageName = packageName.trimmed();
        if (normalizedPackageName.isEmpty()) {
            continue;
        }
        if (!isValidPackageName(normalizedPackageName)) {
            if (valid) {
                *valid = false;
            }
            return QStringList();
        }
        if (!normalizedNames.contains(normalizedPackageName)) {
            normalizedNames.append(normalizedPackageName);
        }
    }
    return normalizedNames;
}

// 通过 SDK 查询软件包安装状态
bool isPackageInstalledWithSdk(const QString &packageName)
{
    ensurePackageManagerEnvironment();

    const QString normalizedPackageName = packageName.trimmed();
    if (normalizedPackageName.isEmpty() || !isValidPackageName(normalizedPackageName)) {
        return false;
    }

    const PackageNameInfo packageInfo = packageNameInfo(normalizedPackageName);
    const QByteArray packageNameBytes = packageInfo.name.toUtf8();
    const QByteArray packageVersionBytes = packageInfo.version.toUtf8();
    return kdk_package_is_installed(packageNameBytes.constData(),
                                    packageInfo.version.isEmpty()
                                        ? nullptr
                                        : packageVersionBytes.constData()) == 0;
}

// 转发 SDK 安装进度回调
void packageInstallCallback(int progress, int state, char *logInfo)
{
    QPointer<SysdbusRegister> receiver;
    QString requestId;
    QString packageName;
    {
        QMutexLocker locker(&g_packageInstallCallbackMutex);
        receiver = g_packageInstallCallbackReceiver;
        requestId = g_packageInstallCallbackRequestId;
        packageName = g_packageInstallCallbackPackageName;
    }

    const QString installLog = logInfo ? QString::fromLocal8Bit(logInfo) : QString();
    qInfo() << "system package install progress:" << progress
            << "state:" << state
            << "request:" << requestId
            << "package:" << packageName
            << "log:" << installLog;

    if (!receiver) {
        return;
    }

    QMetaObject::invokeMethod(receiver.data(),
                              "packageInstallProgress",
                              Qt::QueuedConnection,
                              Q_ARG(QString, requestId),
                              Q_ARG(QString, packageName),
                              Q_ARG(int, progress),
                              Q_ARG(int, state),
                              Q_ARG(QString, installLog));
}

// 设置安装回调接收对象
void setPackageInstallCallbackReceiver(SysdbusRegister *receiver,
                                       const QString &requestId,
                                       const QString &packageName)
{
    QMutexLocker locker(&g_packageInstallCallbackMutex);
    g_packageInstallCallbackReceiver = receiver;
    g_packageInstallCallbackRequestId = requestId;
    g_packageInstallCallbackPackageName = packageName;
}

// 清理安装回调接收对象
void clearPackageInstallCallbackReceiver(SysdbusRegister *receiver,
                                         const QString &requestId = QString())
{
    QMutexLocker locker(&g_packageInstallCallbackMutex);
    if (g_packageInstallCallbackReceiver == receiver
            && (requestId.isEmpty() || g_packageInstallCallbackRequestId == requestId)) {
        g_packageInstallCallbackReceiver.clear();
        g_packageInstallCallbackRequestId.clear();
        g_packageInstallCallbackPackageName.clear();
    }
}

// 转发 SDK 卸载进度回调
void packageUninstallCallback(int progress, int state, char *logInfo)
{
    QPointer<SysdbusRegister> receiver;
    QString requestId;
    QString packageName;
    {
        QMutexLocker locker(&g_packageUninstallCallbackMutex);
        receiver = g_packageUninstallCallbackReceiver;
        requestId = g_packageUninstallCallbackRequestId;
        packageName = g_packageUninstallCallbackPackageName;
    }

    const QString uninstallLog = logInfo ? QString::fromLocal8Bit(logInfo) : QString();
    qInfo() << "system package uninstall progress:" << progress
            << "state:" << state
            << "request:" << requestId
            << "package:" << packageName
            << "log:" << uninstallLog;

    if (!receiver) {
        return;
    }

    QMetaObject::invokeMethod(receiver.data(),
                              "packageUninstallProgress",
                              Qt::QueuedConnection,
                              Q_ARG(QString, requestId),
                              Q_ARG(QString, packageName),
                              Q_ARG(int, progress),
                              Q_ARG(int, state),
                              Q_ARG(QString, uninstallLog));
}

// 设置卸载回调接收对象
void setPackageUninstallCallbackReceiver(SysdbusRegister *receiver,
                                         const QString &requestId,
                                         const QString &packageName)
{
    QMutexLocker locker(&g_packageUninstallCallbackMutex);
    g_packageUninstallCallbackReceiver = receiver;
    g_packageUninstallCallbackRequestId = requestId;
    g_packageUninstallCallbackPackageName = packageName;
}

// 清理卸载回调接收对象
void clearPackageUninstallCallbackReceiver(SysdbusRegister *receiver,
                                           const QString &requestId = QString())
{
    QMutexLocker locker(&g_packageUninstallCallbackMutex);
    if (g_packageUninstallCallbackReceiver == receiver
            && (requestId.isEmpty() || g_packageUninstallCallbackRequestId == requestId)) {
        g_packageUninstallCallbackReceiver.clear();
        g_packageUninstallCallbackRequestId.clear();
        g_packageUninstallCallbackPackageName.clear();
    }
}

// 通过 SDK 安装单个软件包
SystemPackageInstallResult installPackageWithSdk(const QString &packageName,
                                                 const QString &requestId,
                                                 const QPointer<SysdbusRegister> &receiver)
{
    ensurePackageManagerEnvironment();

    SystemPackageInstallResult result;
    result.requestId = requestId;
    result.packageName = packageName.trimmed();
    if (result.packageName.isEmpty() || isPackageInstalledWithSdk(result.packageName)) {
        result.success = true;
        return result;
    }

    qInfo() << Q_FUNC_INFO << "install system package:" << result.packageName
            << "request:" << result.requestId;
    const QByteArray packageNameBytes = result.packageName.toUtf8();
    setPackageInstallCallbackReceiver(receiver.data(), result.requestId, result.packageName);
    const int installResult = kdk_package_install_package_online(packageNameBytes.constData(),
                                                                KDK_PACKAGE_AUTO,
                                                                packageInstallCallback);
    kdk_package_unregister_install_package_handle();
    clearPackageInstallCallbackReceiver(receiver.data(), result.requestId);

    if (isPackageInstalledWithSdk(result.packageName)) {
        result.success = true;
        return result;
    }

    qWarning() << Q_FUNC_INFO << "failed to install system package:"
               << result.packageName << "request:" << result.requestId
               << "result:" << installResult;
    return result;
}

// 通过 SDK 卸载单个软件包
SystemPackageInstallResult uninstallPackageWithSdk(const QString &packageName,
                                                   const QString &requestId,
                                                   const QPointer<SysdbusRegister> &receiver)
{
    ensurePackageManagerEnvironment();

    SystemPackageInstallResult result;
    result.requestId = requestId;
    result.packageName = packageName.trimmed();
    if (result.packageName.isEmpty() || !isPackageInstalledWithSdk(result.packageName)) {
        result.success = true;
        return result;
    }

    qInfo() << Q_FUNC_INFO << "uninstall system package:" << result.packageName
            << "request:" << result.requestId;
    const PackageNameInfo packageInfo = packageNameInfo(result.packageName);
    const QByteArray packageNameBytes = packageInfo.name.toUtf8();
    setPackageUninstallCallbackReceiver(receiver.data(), result.requestId, result.packageName);
    const int uninstallResult = kdk_package_remove_package(packageNameBytes.constData(),
                                                          packageUninstallCallback);
    kdk_package_unregister_remove_package_handle();
    clearPackageUninstallCallbackReceiver(receiver.data(), result.requestId);

    if (!isPackageInstalledWithSdk(result.packageName)) {
        result.success = true;
        return result;
    }

    qWarning() << Q_FUNC_INFO << "failed to uninstall system package:"
               << result.packageName << "request:" << result.requestId
               << "result:" << uninstallResult;
    return result;
}

// 通过 SDK 批量安装软件包
QVector<SystemPackageInstallResult> installPackagesWithSdk(
        const QStringList &packageNames,
        const QString &requestId,
        const QPointer<SysdbusRegister> &receiver)
{
    QVector<SystemPackageInstallResult> results;
    results.reserve(packageNames.size());
    for (const QString &packageName : packageNames) {
        results.append(installPackageWithSdk(packageName, requestId, receiver));
    }
    return results;
}

// 通过 SDK 批量卸载软件包
QVector<SystemPackageInstallResult> uninstallPackagesWithSdk(
        const QStringList &packageNames,
        const QString &requestId,
        const QPointer<SysdbusRegister> &receiver)
{
    QVector<SystemPackageInstallResult> results;
    results.reserve(packageNames.size());
    for (const QString &packageName : packageNames) {
        results.append(uninstallPackageWithSdk(packageName, requestId, receiver));
    }
    return results;
}
} // namespace

SysdbusRegister::SysdbusRegister()
    : UKUI::ServiceObject(nullptr), QDBusContext()
{
    // 构造函数里连接一次即可，不要每次调用接口都连
    QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager",
        "PrepareForSleep", this, SLOT(prepareForSleepSlot(bool)));

    QString aptSettingFile = "/etc/ukui-control-center/apt.conf";
    aptSettings = new QSettings(aptSettingFile, QSettings::IniFormat, this);

    mHibernateFile = "/etc/systemd/sleep.conf";
    mHibernateSet = new QSettings(mHibernateFile, QSettings::IniFormat, this);

    mHibernateSet->setIniCodec("UTF-8");
    exitFlag = false;
    if (getCpuInfo().contains("D2000", Qt::CaseInsensitive)) {
        toGetDisplayInfo = false;
    } else {
        toGetDisplayInfo = true;
    }

    if (getCpuInfo() == QString("11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz")) {
        forceI2c = true;
    } else {
        forceI2c = false;
    }

    _id = 0;

    const char *sessionType = kdk_system_get_production_line();
    if (!QString(sessionType).compare("wayland", Qt::CaseInsensitive)) {
        m_isWayland = true;
    }
    m_diskInfo.clear();
    getDiskInfofromDbus();
}

void SysdbusRegister::onDiskInfoReceivedSlot(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QString> reply = *watcher;
    watcher->deleteLater();
    if (reply.isError()) {
        qWarning() << "Failed to get disk info:" << reply.error();
        m_isRequesting = false;
        return;
    }

    QString diskStr = reply.value();
    qDebug() << Q_FUNC_INFO << "getDiskInfo value" << diskStr;
    if (diskStr.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "getDiskInfo value is empty";
        m_isRequesting = false;
        return;
    }
    QByteArray jsonData = diskStr.toUtf8();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        qDebug() << Q_FUNC_INFO << "diskStr to QJsonDocument is null";
        m_isRequesting = false;
        return;
    }

    if (!doc.isArray()) {
        qDebug() << Q_FUNC_INFO << "diskStr to QJsonDocument is not array";
        m_isRequesting = false;
        return;
    }

    QJsonArray devices = doc.array();
    for (int idx = 0; idx < devices.size(); ++idx) {
        if (!devices[idx].isObject()) {
            qDebug() << Q_FUNC_INFO << "index" << idx << "is not Object";
            continue;
        }
        QJsonObject device = devices[idx].toObject();
        if (device.contains("size")) {
            m_diskInfo = device.value("size").toString();
            qDebug() << Q_FUNC_INFO << "disk size" << m_diskInfo << "emit signal diskInfoReady";
            m_isRequesting = false;
            emit diskInfoReady(m_diskInfo);
            break;
        }
    }
    if (m_diskInfo.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "disk size is empty" << m_diskInfo;
        m_isRequesting = false;
        return;
    }
}

void SysdbusRegister::getDiskInfofromDbus() {
    qDebug() << Q_FUNC_INFO << "dbus is requesting" << m_isRequesting;
    if (m_isRequesting) {
        qDebug() << Q_FUNC_INFO << "dbus is requesting, skip";
        return;
    }
    m_isRequesting = true;
    FreedesktopHelper* freedesktopHelper = FreedesktopHelper::getInstance(false, this);
    if (!freedesktopHelper) {
        qDebug() << Q_FUNC_INFO << "FreedesktopHelper getInstance error, skip";
        return;
    }
    if (!freedesktopHelper->NameHasOwner("com.kylin-os-manager")) {
        qDebug() << Q_FUNC_INFO << "freedesktopHelper NameHasOwner com.kylin-os-manager false";
        connect(freedesktopHelper, &FreedesktopHelper::serviceStatusChanged, this, [this](const QString &name, bool noEmpty) {
            serviceStatusChangedSlot(name, noEmpty);
        });
    } else {
        qDebug() << Q_FUNC_INFO << "freedesktopHelper NameHasOwner com.kylin-os-manager true";
        asynGetDiskInfo();
    }
}

void SysdbusRegister::serviceStatusChangedSlot(const QString &name, bool noEmpty) {
    if (name == "com.kylin-os-manager" && noEmpty) {
        qDebug() << Q_FUNC_INFO << "com.kylin-os-manager is ready";
        QTimer::singleShot(1000, this, [=]() {
            asynGetDiskInfo();
        });
    }
}

void SysdbusRegister::asynGetDiskInfo() {
    qDebug() << Q_FUNC_INFO << "asynGetDiskInfo start";
    QDBusInterface ifc("com.kylin-os-manager",
                       "/driverinfo",
                       "driverinfo.tool",
                       QDBusConnection::systemBus());
    QDBusPendingCall pendingCall = ifc.asyncCall("getDiskInfo");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        onDiskInfoReceivedSlot(watcher);
    });
}

QString SysdbusRegister::getDiskInfo() {
    if (m_diskInfo.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "getDiskInfo is empty, request again" << m_diskInfo << m_isRequesting;
        getDiskInfofromDbus();
        qDebug() << Q_FUNC_INFO << "getDiskInfo is empty, after request again" << m_diskInfo;
    }
    qDebug() << Q_FUNC_INFO << "diskInfo" << m_diskInfo;
    return m_diskInfo;
}

SysdbusRegister::~SysdbusRegister()
{
    exitFlag = true;

}

bool SysdbusRegister::updateGrub(const QString &lang)
{
    //密码校验
    if (!callerPathCompare("/usr/bin/ukui-session-service")) {
        if (!authoriyGrub()){
            return false;
        }
    }

    auto *watcher  = new QFutureWatcher<int>();
    connect(watcher, &QFutureWatcher<int>::finished, this, [this, watcher](){
        int ret = watcher->result();
        qInfo() << "reply of update watcher " << ret;
        watcher->deleteLater();
    });

    QString cmd = QString("export LANG=%1 && update-grub").arg(lang);

    // Start the computation.
    QFuture<int> future = QtConcurrent::run([=, cmd] {
        int ret = system(cmd.toLatin1().data());
        qInfo() << "reply of update" << ret;
        return ret;
    });
    watcher->setFuture(future);

    return true;
}

void SysdbusRegister::exitService()
{
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return ;
    }

    exitFlag = true;
    this->emitUkuiServiceObjectFinished();
}

int SysdbusRegister::setPid(qint64 id)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriySetPid(conn.interface()->servicePid(msg.service()).value())){
       return 0;
    }

    _id = id;

    return 1;
}

bool SysdbusRegister::isSudoGroupNumber(QString uname)
{
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return false;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");

    if (users.contains(uname)){
        return true;
    } else {
        return false;
    }
}

QString SysdbusRegister::GetComputerInfo() {

    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return "";
    }

    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "dmidecode -t system");

    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        while (!feof(fp)) {
            fgets(buf, sizeof (buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    }
    return QString(ba);
}

//获取免密登录状态
QString SysdbusRegister::getNoPwdLoginStatus(){
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/group |grep nopasswdlogin");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    }else{
        qDebug()<<"popen文件打开失败"<<endl;
    }
    return QString(ba);
}

void SysdbusRegister::notifyPropertyChanged( const QString& interface,
                            const QString& propertyName)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        "/",
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged");
    signal << interface;
    QVariantMap changedProps;
    changedProps.insert(propertyName, property(propertyName.toLatin1().data()));
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::systemBus().send(signal);
}

bool SysdbusRegister::NoPwdLoginStatus() const
{
    return NoPwdLogin;
}
//设置免密登录状态
bool SysdbusRegister::setNoPwdLoginStatus(bool status,QString username)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return false;
    }

    if (username == nullptr) {
        if (status == false) {
            QString noPwdLoginUser = getNoPwdLoginStatus();
            qDebug() << "noPwdLoginUser:" << noPwdLoginUser;
            QStringList tmp = noPwdLoginUser.split(":", QString::SkipEmptyParts);

            QString noPasswdUsers = tmp.at(tmp.count()-1);
            QStringList noPasswdUsersList = noPasswdUsers.split(",", QString::SkipEmptyParts);
            foreach (QString noPasswdUser, noPasswdUsersList) {
                noPasswdUser.remove(QChar('\n'), Qt::CaseInsensitive);
                qDebug() << "nopasswduser:" << noPasswdUser;
                QString cmd = QString("gpasswd  -d %1 nopasswdlogin").arg(noPasswdUser);;
                QProcess::execute(cmd);
            }
        }
    } else {
        QString cmd;
        if(true == status){
             cmd = QString("gpasswd  -a %1 nopasswdlogin").arg(username);
        } else{
            cmd = QString("gpasswd  -d %1 nopasswdlogin").arg(username);
        }
        QProcess::execute(cmd);
    }

    NoPwdLogin = status;
    notifyPropertyChanged("com.control.center.qt.systemdbus", "NoPwdLoginStatus");
    return true;
}

// 设置自动登录状态
int SysdbusRegister::setAutoLoginStatus(QString username) {
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriyAutoLogin(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }
    QString filename = "/etc/lightdm/lightdm.conf";
    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
    autoSettings->beginGroup("SeatDefaults");

    autoSettings->setValue("autologin-user", username);

    autoSettings->endGroup();
    autoSettings->sync();

    return 1;
}

QString SysdbusRegister::getSuspendThenHibernate() {
    mHibernateSet->beginGroup("Sleep");

    QString time = mHibernateSet->value("HibernateDelaySec").toString();

    mHibernateSet->endGroup();
    mHibernateSet->sync();

    return time;
}

void SysdbusRegister::setSuspendThenHibernate(QString time) {
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriyAutoLogin(conn.interface()->servicePid(msg.service()).value())){
        return ;
    }

    mHibernateSet->beginGroup("Sleep");

    mHibernateSet->setValue("HibernateDelaySec", time);

    mHibernateSet->endGroup();
    mHibernateSet->sync();
}

int SysdbusRegister::changeOtherUserPasswd(QString username, QString pwd)
{
    return -1;
}

int SysdbusRegister::createUser(QString name, QString fullname, int accounttype, QString faceicon, QString pwd)
{
    return -1;
}

bool SysdbusRegister::checkAuthorization(qint64 id, QString action){

    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                action,
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool SysdbusRegister::authoriySetPid(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.pid",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool SysdbusRegister::authoriyGrub()
{
    QDBusConnection conn = connection();
    QDBusMessage msg = QDBusContext::message();

    _id = conn.interface()->servicePid(msg.service()).value();

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.grub",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool SysdbusRegister::authoriyAutoLogin(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.autologin",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool SysdbusRegister::authoriySetHostName(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.hostname",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    qDebug() << "result = " << result;
    if (result == PolkitQt1::Authority::Yes){
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}
QString SysdbusRegister::getCpuInfo()
{
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
        result.append(modelLine.first().split(':').at(1));
        result = result.trimmed();

        return result;
    }

    return QString();
}


int SysdbusRegister::changeRTC() {
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    QString cmd = "hwclock -w";
    return system(cmd.toLatin1().data());
}

bool SysdbusRegister::authoriySetNtpServer(qint64 id)
{
    _id = id;

    if (_id == 0) {
        return false;
    }

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.ntp",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    qDebug() << "authoriySetNtpServer result =" << result;
    if (result == PolkitQt1::Authority::Yes) {
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

// 校验软件包安装权限
bool SysdbusRegister::authoriyInstallPackage(qint64 id)
{
    _id = id;

    if (_id == 0) {
        return false;
    }

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.packageinstall",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    qDebug() << "authoriyInstallPackage result =" << result;
    if (result == PolkitQt1::Authority::Yes) {
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

// 校验软件包卸载权限
bool SysdbusRegister::authoriyUninstallPackage(qint64 id)
{
    _id = id;

    if (_id == 0) {
        return false;
    }

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.packageuninstall",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    qDebug() << "authoriyUninstallPackage result =" << result;
    if (result == PolkitQt1::Authority::Yes) {
        _id = 0;
        return true;
    } else {
        _id = 0;
        return false;
    }
}

bool SysdbusRegister::setNtpSerAddress(QString serverAddress)
{
    qDebug() << Q_FUNC_INFO << "start setNtpSerAddress" << serverAddress;
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    qDebug() << Q_FUNC_INFO << "start setNtpSerAddress id" << conn.interface()->servicePid(msg.service()).value();
    if (!authoriySetNtpServer(conn.interface()->servicePid(msg.service()).value())) {
        qDebug() << Q_FUNC_INFO << "setNtpSerAddress polkit authority failed";
        return false;
    } else {
        qDebug() << Q_FUNC_INFO << "setNtpSerAddress polkit authority suc";
    }

    if (serverAddress == "default") {
        system("rm -rf /etc/systemd/timesyncd.conf.d/");
        system("timedatectl set-ntp false");
        system("timedatectl set-ntp true");
        qDebug() << Q_FUNC_INFO << "emit ntpServerChangedDefault";
        emit ntpServerChangedDefault();
        return true;
    }

    QString dirName  = "/etc/systemd/timesyncd.conf.d/";
    QString fileName = "/etc/systemd/timesyncd.conf.d/kylin.conf";

    QDir  dir(dirName);
    QFile file(fileName);
    if (!dir.exists()) {
        if (dir.mkdir(dirName) == false) {
            qDebug() << Q_FUNC_INFO << "setNtpSerAddress mkdir dirName failed" << dirName;
            return false;
        }
    }
    if (file.open(QIODevice::WriteOnly) == false) {
        qDebug() << Q_FUNC_INFO << "setNtpSerAddress open file failed" << dirName;
        return false;
    }
    file.write("[Time]\n");
    file.write("NTP = ");
    file.write(serverAddress.toLatin1().data());
    file.write("\n");
    file.close();
    qDebug() << Q_FUNC_INFO << "setNtpSerAddress timedatectl set-ntp" << serverAddress;
    system("timedatectl set-ntp false");
    system("timedatectl set-ntp true");
    return true;

}

bool SysdbusRegister::setaptproxy(QString ip, QString port, bool open)
{
    return setaptproxy(ip, port, ip, port, open);
}

bool SysdbusRegister::setaptproxy(QString httpIp, QString httpPort, QString httpsIp, QString httpsPort, bool open)
{
    qDebug() << "proxy IP/hostname or port:"<< httpIp << httpPort << httpsIp << httpsPort <<open;
    // 参数安全校验（防止命令注入）
    // 1. 主机名/IP 正则：符合域名规范或标准 IPv4
    static const QRegularExpression hostRegex(
        "^(?=.{1,253}$)(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])\\.)*"
        "([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])$"
    );
    static const QRegularExpression ipv4Regex(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
    );
    // 2. 端口正则：1-65535
    static const QRegularExpression portRegex(
        "^([1-9][0-9]{0,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"
    );

    auto isValidHost = [&](const QString& host) {
        if(host.trimmed().isEmpty()){ return true;}// 留空
        return hostRegex.match(host).hasMatch() || ipv4Regex.match(host).hasMatch();
    };
    auto isValidPort = [&](const QString& port) {
        if(port.trimmed().isEmpty()){ return true;}// 留空
        return portRegex.match(port).hasMatch();
    };

    if (!isValidHost(httpIp) || !isValidPort(httpPort) ||
        !isValidHost(httpsIp) || !isValidPort(httpsPort)) {
        qDebug() << "Invalid proxy IP/hostname or port:"
                   << httpIp << httpPort << httpsIp << httpsPort;
        return false;
    }

    QStringList keys = aptSettings->childGroups();
    aptSettings->beginGroup("Info");
    aptSettings->setValue("open", open);
    aptSettings->setValue("http_ip", httpIp);
    aptSettings->setValue("http_port", httpPort);
    aptSettings->setValue("https_ip", httpsIp);
    aptSettings->setValue("https_port", httpsPort);
    aptSettings->endGroup();
    QString content_http = QString("%1%2%3%4%5%6").arg("Acquire::http::Proxy ").arg("\"http://").arg(httpIp).arg(":").arg(httpPort).arg("\";\n");
    QString content_https = QString("%1%2%3%4%5%6").arg("Acquire::https::Proxy ").arg("\"http://").arg(httpsIp).arg(":").arg(httpsPort).arg("\";\n");
    QString profile_http = QString("%1%2%3%4%5").arg("export http_proxy=\"http://").arg(httpIp).arg(":").arg(httpPort).arg("\"\n");
    QString profile_https = QString("%1%2%3%4%5").arg("export https_proxy=\"http://").arg(httpsIp).arg(":").arg(httpsPort).arg("\"\n");
    QString dirName  = "/etc/apt/apt.conf.d/";
    QString fileName = "/etc/apt/apt.conf.d/80apt-proxy";
    QString dirName_1  = "/etc/profile.d/";
    QString fileName_1 = "/etc/profile.d/80apt-proxy.sh";
    QDir AptDir(dirName);
    QDir ProDir(dirName_1);
    QFile AptProxyFile(fileName);
    QFile AptProxyProFile(fileName_1);
    if (AptDir.exists() && ProDir.exists()) {
        if (open) {    //开关开启则创建对应文件，未开启则删掉对应文件
            if (AptProxyFile.exists() && AptProxyProFile.exists()) {
               AptProxyFile.remove();
               AptProxyProFile.remove();
            }
            AptProxyFile.open(QIODevice::ReadWrite | QIODevice::Text);
            AptProxyProFile.open(QIODevice::ReadWrite | QIODevice::Text);
            //写入内容,这里需要转码，否则报错
            QByteArray str = content_http.toUtf8();
            QByteArray str_1 = content_https.toUtf8();
            QByteArray str_2 = profile_http.toUtf8();
            QByteArray str_3 = profile_https.toUtf8();
            //写入QByteArray格式字符串
            AptProxyFile.write(str);
            AptProxyFile.write(str_1);
            AptProxyProFile.write(str_2);
            AptProxyProFile.write(str_3);
        } else {
            if (AptProxyFile.exists() && AptProxyProFile.exists()) {
               AptProxyFile.remove();
               AptProxyProFile.remove();
            }
        }
    }else {
           return false;
    }
    return true;
}

QHash<QString, QVariant> SysdbusRegister::getaptproxy()
{
    QHash<QString, QVariant> mAptInfo;
    aptSettings->beginGroup("Info");
    mAptInfo.insert("open" ,  aptSettings->value("open").toBool());
    mAptInfo.insert("http_ip" ,  aptSettings->value("http_ip").toString());
    mAptInfo.insert("http_port" ,  aptSettings->value("http_port").toString());
    mAptInfo.insert("https_ip" ,  aptSettings->value("https_ip").toString());
    mAptInfo.insert("https_port" ,  aptSettings->value("https_port").toString());
    aptSettings->endGroup();
    return mAptInfo;
}

// 在线批量安装软件包
bool SysdbusRegister::installPackagesOnline(QStringList packageNames, QString requestId)
{
    const QString normalizedRequestId = requestId.trimmed();
    bool validPackageNames = false;
    const QStringList normalizedPackageNameList = normalizedPackageNames(packageNames, &validPackageNames);
    if (normalizedRequestId.isEmpty()
            || normalizedPackageNameList.isEmpty()
            || !validPackageNames) {
        qWarning() << Q_FUNC_INFO << "invalid package batch install request:"
                   << normalizedRequestId << packageNames;
        return false;
    }

    if (m_packageInstallRunning || m_packageUninstallRunning) {
        qWarning() << Q_FUNC_INFO << "another package task is running, reject batch install request:"
                   << normalizedRequestId << normalizedPackageNameList;
        return false;
    }

    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!authoriyInstallPackage(conn.interface()->servicePid(msg.service()).value())) {
        qWarning() << Q_FUNC_INFO << "polkit authority failed:"
                   << normalizedRequestId << normalizedPackageNameList;
        return false;
    }

    bool hasPackageToInstall = false;
    for (const QString &packageName : normalizedPackageNameList) {
        if (!isPackageInstalledWithSdk(packageName)) {
            hasPackageToInstall = true;
            break;
        }
    }
    if (!hasPackageToInstall) {
        for (const QString &packageName : normalizedPackageNameList) {
            Q_EMIT packageInstallProgress(normalizedRequestId, packageName, 100, 0, QString());
            Q_EMIT packageInstallFinished(normalizedRequestId, packageName, true, QString());
        }
        return true;
    }

    m_packageInstallRunning = true;
    auto *watcher = new QFutureWatcher<QVector<SystemPackageInstallResult>>(this);
    connect(watcher, &QFutureWatcher<QVector<SystemPackageInstallResult>>::finished, this, [this, watcher]() {
        const QVector<SystemPackageInstallResult> results = watcher->result();
        watcher->deleteLater();
        m_packageInstallRunning = false;

        for (const SystemPackageInstallResult &result : results) {
            if (result.success) {
                Q_EMIT packageInstallProgress(result.requestId, result.packageName, 100, 0, QString());
            }
            Q_EMIT packageInstallFinished(result.requestId,
                                          result.packageName,
                                          result.success,
                                          result.errorText);
        }
    });

    QPointer<SysdbusRegister> receiver(this);
    QFuture<QVector<SystemPackageInstallResult>> future = QtConcurrent::run([=]() {
        return installPackagesWithSdk(normalizedPackageNameList, normalizedRequestId, receiver);
    });
    watcher->setFuture(future);
    return true;
}

// 批量卸载软件包
bool SysdbusRegister::uninstallPackages(QStringList packageNames, QString requestId)
{
    const QString normalizedRequestId = requestId.trimmed();
    bool validPackageNames = false;
    const QStringList normalizedPackageNameList = normalizedPackageNames(packageNames, &validPackageNames);
    if (normalizedRequestId.isEmpty()
            || normalizedPackageNameList.isEmpty()
            || !validPackageNames) {
        qWarning() << Q_FUNC_INFO << "invalid package batch uninstall request:"
                   << normalizedRequestId << packageNames;
        return false;
    }

    if (m_packageInstallRunning || m_packageUninstallRunning) {
        qWarning() << Q_FUNC_INFO << "another package task is running, reject batch uninstall request:"
                   << normalizedRequestId << normalizedPackageNameList;
        return false;
    }

    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!authoriyUninstallPackage(conn.interface()->servicePid(msg.service()).value())) {
        qWarning() << Q_FUNC_INFO << "polkit authority failed:"
                   << normalizedRequestId << normalizedPackageNameList;
        return false;
    }

    bool hasPackageToRemove = false;
    for (const QString &packageName : normalizedPackageNameList) {
        if (isPackageInstalledWithSdk(packageName)) {
            hasPackageToRemove = true;
            break;
        }
    }
    if (!hasPackageToRemove) {
        for (const QString &packageName : normalizedPackageNameList) {
            Q_EMIT packageUninstallProgress(normalizedRequestId, packageName, 100, 0, QString());
            Q_EMIT packageUninstallFinished(normalizedRequestId, packageName, true, QString());
        }
        return true;
    }

    m_packageUninstallRunning = true;
    auto *watcher = new QFutureWatcher<QVector<SystemPackageInstallResult>>(this);
    connect(watcher, &QFutureWatcher<QVector<SystemPackageInstallResult>>::finished, this, [this, watcher]() {
        const QVector<SystemPackageInstallResult> results = watcher->result();
        watcher->deleteLater();
        m_packageUninstallRunning = false;

        for (const SystemPackageInstallResult &result : results) {
            if (result.success) {
                Q_EMIT packageUninstallProgress(result.requestId, result.packageName, 100, 0, QString());
            }
            Q_EMIT packageUninstallFinished(result.requestId,
                                            result.packageName,
                                            result.success,
                                            result.errorText);
        }
    });

    QPointer<SysdbusRegister> receiver(this);
    QFuture<QVector<SystemPackageInstallResult>> future = QtConcurrent::run([=]() {
        return uninstallPackagesWithSdk(normalizedPackageNameList, normalizedRequestId, receiver);
    });
    watcher->setFuture(future);
    return true;
}

bool SysdbusRegister::sethostname(QString hostname)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriySetHostName(conn.interface()->servicePid(msg.service()).value())){
        return false;
    }

    QString fileName = "/etc/hosts";
    QString strAll;
    QFile readFile(fileName);
    if(readFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream stream_1(&readFile);
        int count = 0;
        while(!stream_1.atEnd())
        {
            if (count != 0)
                strAll.append("\n");
            count++;
            QString oneLine = stream_1.readLine();  //读取一行
            if(oneLine.contains("127.0.1.1")) {
                QString tempStr = QString("%1%2").arg("127.0.1.1       ").arg(hostname);
                strAll.append(tempStr);
                continue;
            }
            strAll.append(oneLine);
        }
    }
    readFile.close();
    QFile writeFile(fileName);
    if(writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
            QTextStream stream_2(&writeFile);
            stream_2<<strAll;
    }
    writeFile.close();

    return true;
}

QString SysdbusRegister::getMemory()
{
    float memorysize(0.0);
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("bash" , QStringList() << "-c" << "dmidecode -t memory | grep Size ");
    process->waitForFinished();
    QByteArray ba = process->readAllStandardOutput();
    delete process;

    QString sizeinfo = QString(ba.data());
    QStringList size_list = sizeinfo.split('\n');
    for (QString str : size_list) {
        str.remove(QRegExp("\\s"));
        if (str.split(':').at(0) == "Size") {
            QString res = str.split(':').at(1);
            QRegExp rx("^\\d");
            if (rx.indexIn(res) == 0){
                QRegExp rx_1("^(.*)MB$");
                QRegExp rx_2("^(.*)GB$");
                int pos_1 = rx_1.indexIn(res);
                int pos_2 = rx_2.indexIn(res);
                if (pos_1 > -1) {
                    qDebug()<<rx_1.cap(1);
                    memorysize +=  ceil(rx_1.cap(1).toFloat()/1024);
                }
                if (pos_2 > -1) {
                    qDebug()<<rx_2.cap(1);
                    memorysize +=  rx_2.cap(1).toFloat();
                }
            }
        }
    }
    qDebug()<<"memory : "<<memorysize;
    return QString::number(memorysize);
}

void SysdbusRegister::getDisplayInfo() {}

void SysdbusRegister::setDisplayBrightness(QString brightness, QString edidHash, QString i2cBus) {}

int SysdbusRegister::getDisplayBrightness(QString edidHash, QString i2cBus)
{
    return -2;
}

QString SysdbusRegister::showDisplayInfo()
{
    QString retString = "";
    return retString;
}

QString SysdbusRegister::getDmidecodeType()
{
    QString retString = "";
    QProcess process;
    QString cmd = "dmidecode --type 3 | grep Type:";
    process.start("bash", QStringList() <<"-c" << cmd);
    process.waitForFinished();
    QString strResult = process.readAllStandardOutput()+process.readAllStandardError();
    if (strResult.contains("Desktop", Qt::CaseInsensitive)) {
        retString = "desktop";
    } else if (strResult.contains("All In One", Qt::CaseInsensitive)) {
        retString = "all in one";
    } else if (strResult.contains("Notebook", Qt::CaseInsensitive)) {
        retString = "notebook";
    } else {
        retString = "unknown";
    }

    return retString;
}

QString SysdbusRegister::getDmiDecodeRes(QString args)
{
    if (args == "system-product-name") {
        args = "-s " + args;
    } else if (args == "baseboard") {
        args = "-t " + args;
    } else {
        return "";
    }

    QString program = "/usr/sbin/dmidecode " + args;

    QProcess *vcpPro = new QProcess(this);
    vcpPro->start(program);
    vcpPro->waitForFinished();
    QByteArray ba = vcpPro->readAllStandardOutput();
    vcpPro->deleteLater();
    return QString::fromStdString(ba.toStdString());
}

QString SysdbusRegister::getShowTimezone()
{
    QFile file("/etc/.kytimezone");
    QString mTimezone = "";
    if(file.exists()) {
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream textStream(&file);
            mTimezone = textStream.readLine();
            file.close();
        }
    }
    return mTimezone;
}

bool SysdbusRegister::setShowTimezone(QString timezone)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();


    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value(),
                            "org.control.center.qt.systemdbus.action.timezone")){
        return false;
    }

    QDBusInterface timeDateInterface("org.freedesktop.timedate1",
                                     "/org/freedesktop/timedate1",
                                     "org.freedesktop.timedate1",
                                      QDBusConnection::systemBus());

    const QStringList LINK_SHANGHAI_TIMEZONE = {
        "Asia/Beijing",
        "Asia/Urumqi"
    };
    QString setTimezone = timezone;
    if (LINK_SHANGHAI_TIMEZONE.contains(timezone))
        setTimezone = "Asia/Shanghai";
    QDBusMessage retDBus = timeDateInterface.call("SetTimezone", setTimezone, true);
    if (retDBus.type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    QFile file("/etc/.kytimezone");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                QIODevice::Text)) {
        qDebug() << "setshowtimezone faild" << timezone;
        return false;
    }
    file.write(timezone.toStdString().c_str());

    Q_EMIT changed("timezone");
    return true;
}

bool SysdbusRegister::setGrupPasswd(QString username, QString passwd, QString lang, bool status)
{
    //密码校验
    if (!authoriyGrub()){
        return false;
    }

    // 统一设置环境变量 LANG，所有子进程自动继承
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", lang);

    // 辅助 lambda：执行命令并返回退出码
    auto run = [&](const QString &program, const QStringList &args) -> int {
        QProcess p;
        p.setProcessEnvironment(env);
        p.start(program, args);
        if (!p.waitForFinished(-1)) {
            qDebug() << program << "failed to start or crashed";
            return -1;
        }
        int code = p.exitCode();
        if (code != 0) {
            qDebug() << program << "exit code:" << code << "stderr:" << p.readAllStandardError();
        }
        return code;
    };

    // ========== grub-password ==========
    int ret;
    if (status) {
        ret = run("grub-password", QStringList() << "-u" << username << passwd);
    } else {
        ret = run("grub-password", QStringList() << "-d");
    }
    if (ret != 0 && !m_isWayland) {
        return false;
    }

    // ========== Wayland 分支：先 remount rw ==========
    if (m_isWayland) {
        run("mount", QStringList() << "-o" << "remount,rw" << "/boot/");
        run("mount", QStringList() << "-o" << "remount,rw" << "/boot/efi");
    }

    // ========== update-grub ==========
    ret = run("update-grub", QStringList());
    if (ret != 0 && !m_isWayland) {
        return false;
    }

    // ========== grub-install ==========
    ret = run("grub-install", QStringList());
    if (ret != 0 && !m_isWayland) {
        return false;
    }

    // ========== Wayland 分支：最后 remount ro（必须执行） ==========
    if (m_isWayland) {
        int ret1 = run("mount", QStringList() << "-o" << "remount,ro" << "/boot/");
        int ret2 = run("mount", QStringList() << "-o" << "remount,ro" << "/boot/efi");
        return (ret1 == 0) && (ret2 == 0);
    }

    return true;
}

bool SysdbusRegister::getGrupPasswdStatus()
{
    QByteArray ba = "";
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /boot/grub/grub.cfg |grep password_pbkdf2");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    }else{
        qDebug()<<"popen文件打开失败"<<endl;
    }

    if (!ba.contains("password_pbkdf2")) {
        return false;
    }

    return true;
}

bool SysdbusRegister::callerPathCompare(QString path)
{
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int  pid = intf->servicePid(clientService).value();
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath = "";
    if (file.exists()) {
        execPath= file.canonicalFilePath();
    }
    if (execPath != path) {
        return false;
    }

    return true;
}

bool SysdbusRegister::setGrupBootTime(int time, QString lang)
{
    //密码校验
    if (!authoriyGrub()){
        return false;
    }

    QString filePath = "/etc/default/grub";
    QString keyword = "GRUB_TIMEOUT=";
    QString replaceContent = QString::number(time);
    QFile fileRead(filePath);       //先读取文件信息，QIODevice::Text能将换行转为\n
    QString arryRead;
    if (fileRead.open(QIODevice::ReadOnly|QIODevice::Text)) {
        arryRead = fileRead.readAll();
    }
    fileRead.close();

    //存储每一段信息
    QStringList arryListWrite = arryRead.split("\n");

    QFile fileWrite(filePath);
    if (fileWrite.open(QIODevice::WriteOnly|QIODevice::Text)) {        //文件流写文件
        QTextStream streamWrite(&fileWrite);      //通过文本流控制文件写操作

        //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
        for (int i = 0 ; i < arryListWrite.size() - 1; i++) {
            QString contentWrite= arryListWrite.at(i);
            if (contentWrite.contains(keyword)) {
                int indexBefore = contentWrite.indexOf(keyword);
                contentWrite.replace(indexBefore + keyword.length(), contentWrite.size() - indexBefore - keyword.length(), replaceContent);
                streamWrite << contentWrite << "\n";
            } else {
                streamWrite << arryListWrite.at(i) << "\n";
            }
        }

        fileWrite.close();

        QString cmd = QString("export LANG=%1 && update-grub && grub-install").arg(lang);
        if (m_isWayland) {
            cmd = QString("mount -o remount,rw /boot/ && mount -o remount,rw /boot/efi && export LANG=%1 && update-grub"
                          "&& grub-install || true && mount -o remount,ro /boot/ && mount -o remount,ro /boot/efi").arg(lang);
        }
        qDebug() << "cmd= " << cmd;

        // Start the computation.
        int ret = system(cmd.toLatin1().data());
        if (ret != 0) {
            return false;
        }
    } else {
        qDebug() << filePath << "open failed";
        return false;
    }

    return true;
}

int SysdbusRegister::getGrubBootTime()
{
    QByteArray ba = "";
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/default/grub | grep GRUB_TIMEOUT | cut -d '=' -f 2");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    } else {
        qDebug()<<"popen文件打开失败"<<endl;
    }

    if (ba.isEmpty()) {
        return -1;
    }

    qDebug() << "GRUB_TIMEOUT = " << ba.toInt();

    return ba.toInt();
}

bool SysdbusRegister::getRescueModeStatus()
{
    QByteArray ba = "";
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/default/grub.d/minimalos.cfg | grep GRUB_BOOTFAIL_ENABLE | cut -d '=' -f 2");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    } else {
        qDebug() << "popen文件打开失败";
    }

    if (ba.isEmpty()) {
        return false;
    }

    qDebug() << "GRUB_BOOTFAIL_ENABLE = " << ba;

    if (ba.contains("true")) {
        return true;
    }

    return false;
}

bool SysdbusRegister::setRescueMode(bool status, QString lang)
{
    //密码校验
    if (!authoriyGrub()){
        return false;
    }

    QString filePath = "/etc/default/grub.d/minimalos.cfg";
    QString keyword = "GRUB_BOOTFAIL_ENABLE=";
    QString replaceContent = status ? "true" : "false";
    QFile fileRead(filePath);       //先读取文件信息，QIODevice::Text能将换行转为\n
    QString arryRead;
    if (fileRead.open(QIODevice::ReadOnly|QIODevice::Text)) {
        arryRead = fileRead.readAll();
    }
    fileRead.close();
    qDebug() << "arryRead" << arryRead;

    //存储每一段信息
    QStringList arryListWrite = arryRead.split("\n");
    qDebug() << "arryListWrite:";
    for(int i = 0; i < arryListWrite.size(); i++){
        qDebug() << arryListWrite.at(i);
    }

    QFile fileWrite(filePath);
    if (fileWrite.open(QIODevice::WriteOnly|QIODevice::Text)) {        //文件流写文件
        QTextStream streamWrite(&fileWrite);      //通过文本流控制文件写操作

        //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
        for (int i = 0 ; i < arryListWrite.size() - 1; i++) {
            QString contentWrite= arryListWrite.at(i);
            if (contentWrite.contains(keyword)) {
                int indexBefore = contentWrite.indexOf(keyword);
                contentWrite.replace(indexBefore + keyword.length(), contentWrite.size() - indexBefore - keyword.length(), replaceContent);
                streamWrite << contentWrite << "\n";
            } else {
                streamWrite << arryListWrite.at(i) << "\n";
            }
        }

        fileWrite.close();
        //grub-install用于将当前系统的grub安装到首选启动grub，从而能够让下次启动时候可以读取本系统的grub.cfg
        //update-grub用于更新本系统的grub.cfg文件，从而使本系统更新的默认启动项和延时等设置能生效
        //wayland下grub-install部分会失败，会导致后面的命令不执行，先执行update-grub 再执行grub-install
        QString cmd = QString("export LANG=%1 && update-grub && grub-install").arg(lang);
        if (m_isWayland) {
            cmd = QString("mount -o remount,rw /boot/ && mount -o remount,rw /boot/efi && export LANG=%1 && update-grub"
                          "&& grub-install || true && mount -o remount,ro /boot/ && mount -o remount,ro /boot/efi").arg(lang);
        }
        qDebug() << "cmd= " << cmd;
        int ret = system(cmd.toLatin1().data());
        if (ret != 0) {
            return false;
        }
    } else {
        qDebug() << filePath << "open failed";
        return false;
    }

    return true;
}

int SysdbusRegister::getFailuresTimes()
{
    QByteArray ba = "";
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/default/grub.d/minimalos.cfg | grep GRUB_BOOTFAIL_LIMIT | cut -d '=' -f 2");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    } else {
        qDebug()<<"popen文件打开失败"<<endl;
    }

    if (ba.isEmpty()) {
        return -1;
    }

    qDebug() << "GRUB_BOOTFAIL_LIMIT = " << ba.toInt();

    return ba.toInt();
}

bool SysdbusRegister::setFailuresTimes(int index, QString lang)
{
    //密码校验
    if (!authoriyGrub()){
        return false;
    }

    QString filePath = "/etc/default/grub.d/minimalos.cfg";
    QString keyword = "GRUB_BOOTFAIL_LIMIT=";
    QString replaceContent = QString::number(index);
    QFile fileRead(filePath);       //先读取文件信息，QIODevice::Text能将换行转为\n
    QString arryRead;
    if (fileRead.open(QIODevice::ReadOnly|QIODevice::Text)) {
        arryRead = fileRead.readAll();
    }
    fileRead.close();
    qDebug() << "arryRead" << arryRead;

    //存储每一段信息
    QStringList arryListWrite = arryRead.split("\n");
    qDebug() << "arryListWrite:";
    for(int i = 0; i < arryListWrite.size(); i++){
        qDebug() << arryListWrite.at(i);
    }

    QFile fileWrite(filePath);
    if (fileWrite.open(QIODevice::WriteOnly|QIODevice::Text)) {        //文件流写文件
        QTextStream streamWrite(&fileWrite);      //通过文本流控制文件写操作

        //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
        for (int i = 0 ; i < arryListWrite.size() - 1; i++) {
            QString contentWrite= arryListWrite.at(i);
            if (contentWrite.contains(keyword)) {
                int indexBefore = contentWrite.indexOf(keyword);
                contentWrite.replace(indexBefore + keyword.length(), contentWrite.size() - indexBefore - keyword.length(), replaceContent);
                streamWrite << contentWrite << "\n";
            } else {
                streamWrite << arryListWrite.at(i) << "\n";
            }
        }

        fileWrite.close();
        //grub-install用于将当前系统的grub安装到首选启动grub，从而能够让下次启动时候可以读取本系统的grub.cfg
        //update-grub用于更新本系统的grub.cfg文件，从而使本系统更新的默认启动项和延时等设置能生效
        //wayland下grub-install部分会失败，会导致后面的命令不执行，先执行update-grub 再执行grub-install
        QString cmd = QString("export LANG=%1 && update-grub && grub-install").arg(lang);
        if (m_isWayland) {
            cmd = QString("mount -o remount,rw /boot/ && mount -o remount,rw /boot/efi && export LANG=%1 && update-grub"
                          "&& grub-install || true && mount -o remount,ro /boot/ && mount -o remount,ro /boot/efi").arg(lang);
        }
        qDebug() << "cmd= " << cmd;
        int ret = system(cmd.toLatin1().data());
        if (ret != 0) {
            return false;
        }
    } else {
        qDebug() << filePath << "open failed";
        return false;
    }

    return true;
}

bool SysdbusRegister::setDefaultBootItem(int index, QString lang)
{
    //密码校验
    if (!authoriyGrub()){
        return false;
    }

    QString filePath = "/etc/default/grub";
    QString keyword = "GRUB_DEFAULT=";
    QString replaceContent = QString::number(index);
    QFile fileRead(filePath);       //先读取文件信息，QIODevice::Text能将换行转为\n
    QString arryRead;
    if (fileRead.open(QIODevice::ReadOnly|QIODevice::Text)) {
        arryRead = fileRead.readAll();
    }
    fileRead.close();
    qDebug() << "arryRead" << arryRead;

    //存储每一段信息
    QStringList arryListWrite = arryRead.split("\n");
    qDebug() << "arryListWrite:";
    for(int i = 0; i < arryListWrite.size(); i++){
        qDebug() << arryListWrite.at(i);
    }

    QFile fileWrite(filePath);
    if (fileWrite.open(QIODevice::WriteOnly|QIODevice::Text)) {        //文件流写文件
        QTextStream streamWrite(&fileWrite);      //通过文本流控制文件写操作

        //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉
        for (int i = 0 ; i < arryListWrite.size() - 1; i++) {
            QString contentWrite= arryListWrite.at(i);
            if (contentWrite.contains(keyword)) {
                int indexBefore = contentWrite.indexOf(keyword);
                contentWrite.replace(indexBefore + keyword.length(), contentWrite.size() - indexBefore - keyword.length(), replaceContent);
                streamWrite << contentWrite << "\n";
            } else {
                streamWrite << arryListWrite.at(i) << "\n";
            }
        }

        fileWrite.close();
        //grub-install用于将当前系统的grub安装到首选启动grub，从而能够让下次启动时候可以读取本系统的grub.cfg
        //update-grub用于更新本系统的grub.cfg文件，从而使本系统更新的默认启动项和延时等设置能生效
        //wayland下grub-install部分会失败，会导致后面的命令不执行，先执行update-grub 再执行grub-install
        QString cmd = QString("export LANG=%1 && update-grub && grub-install").arg(lang);
        if (m_isWayland) {
            cmd = QString("mount -o remount,rw /boot/ && mount -o remount,rw /boot/efi && export LANG=%1 && update-grub"
                          "&& grub-install || true && mount -o remount,ro /boot/ && mount -o remount,ro /boot/efi").arg(lang);
        }
        qDebug() << "cmd= " << cmd;

        // Start the computation.
        int ret = system(cmd.toLatin1().data());
        if (ret != 0) {
            return false;
        }
    } else {
        qDebug() << filePath << "open failed";
        return false;
    }

    return true;
}

int SysdbusRegister::getDefaultBootItem()
{
    QByteArray ba = "";
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/default/grub | grep GRUB_DEFAULT | cut -d '=' -f 2");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    } else {
        qDebug()<<"popen文件打开失败"<<endl;
    }

    if (ba.isEmpty()) {
        return -1;
    }

    qDebug() << "GRUB_TIMEOUT = " << ba.toInt();

    return ba.toInt();
}

void SysdbusRegister::prepareForSleepSlot(bool start)
{
    qDebug() << "status = " << start << __FUNCTION__;
    // true = 即将睡眠，false = 唤醒
    if (!start || !m_proberProcess)
        return;

    if (m_proberProcess->state() != QProcess::NotRunning) {
        m_proberProcess->terminate();           // 先优雅退出
        if (!m_proberProcess->waitForFinished(2000)) {
            m_proberProcess->kill();            // 2秒不退出就强杀
        }
    }
}

int SysdbusRegister::getActualSystem()
{
    QStringList outputLines;

    // 防止上次调用残留
    if (m_proberProcess) {
        m_proberProcess->deleteLater();
    }

    m_proberProcess = new QProcess(this);
    m_proberProcess->start("os-prober");

    // 等待启动
    if (!m_proberProcess->waitForStarted(5000)) {
        m_proberProcess->deleteLater();
        m_proberProcess = nullptr;
        return 1;  // 默认只有本系统
    }

    // 用 QEventLoop 等待进程结束，同时保持事件循环处理 D-Bus 信号
    QEventLoop loop;
    connect(m_proberProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            &loop, &QEventLoop::quit);

    // 30秒超时保护，防止 os-prober 卡死导致接口永远挂起
    QTimer::singleShot(30000, &loop, &QEventLoop::quit);

    loop.exec();

    // 判断是否正常退出（被信号杀掉时 exitStatus() 是 CrashExit）
    if (m_proberProcess->exitStatus() == QProcess::NormalExit
        && m_proberProcess->exitCode() == 0) {
        QString output = QString::fromLocal8Bit(m_proberProcess->readAllStandardOutput());
        outputLines = output.split('\n', Qt::SkipEmptyParts);
    }

    m_proberProcess->deleteLater();
    m_proberProcess = nullptr;

    int actualSystem = 0;
    // 不包含windows
    foreach (QString name, outputLines) {
        if (name.contains("windows")) {
            continue;
        }
        actualSystem++;
    }

    // os-prober 输出不包含本身，获取到的系统个数加一
    actualSystem++;

    qDebug() << "actualSystem = " << actualSystem << outputLines;

    return actualSystem;
}

QString SysdbusRegister::getSysInstallTime(const QString &part)
{
    QProcess process;

    QString cmd = QString("/usr/sbin/tune2fs -l /dev/%1 |grep create").arg(part);
    process.start(cmd);
    process.waitForFinished();
    QString info = process.readAllStandardOutput();
    if (!info.isEmpty()) {
        if (info.contains("Filesystem created")) {
            info.replace(QRegExp("[\\s]+"), " ");
            return info.mid(20);
        }
    }
    return QString();
}

void SysdbusRegister::setXrdpService(bool status)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!callerPathCompare("/usr/bin/ukui-control-center")) {
        if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value()), "org.control.center.qt.systemdbus.action.remote"){
            return;
        }
    }

    QString cmd = QString("systemctl %1 xrdp.service").arg(status ? "start" : "stop");
    system(cmd.toUtf8().data());
    return;
}
