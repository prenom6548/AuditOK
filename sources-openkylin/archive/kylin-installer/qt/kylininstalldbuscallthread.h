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
#ifndef KYLININSTALLDBUSCALLTHREAD_H
#define KYLININSTALLDBUSCALLTHREAD_H

/*#define KYLIN_OS_DBUS_SERVICE_NAME "cn.kylinos.KylinUpdateManager"
#define KYLIN_OS_DBUS_PATH  "/cn/kylinos/KylinUpdateManager"
#define KYLIN_OS_DBUS_INTERFACE_NAME "cn.kylinos.KylinUpdateManager"
*/
// #define KYLIN_OS_DBUS_SERVICE_NAME "com.kylin.systemupgrade"
// #define KYLIN_OS_DBUS_PATH  "/com/kylin/systemupgrade"
// #define KYLIN_OS_DBUS_INTERFACE_NAME "com.kylin.systemupgrade.interface"
 

#define KYLIN_OS_DBUS_SERVICE_NAME "com.kylin.systemupgrade"
#define KYLIN_OS_DBUS_PATH  "/com/kylin/systemupgrade"
#define KYLIN_OS_DBUS_INTERFACE_NAME "com.kylin.systemupgrade.interface"

#define KYLIN_OS_KARE_DBUS_SERVICE_NAME "org.kylin.kare"
#define KYLIN_OS_KARE_DBUS_PATH  "/org/kylin/kare"
#define KYLIN_OS_KARE_DBUS_INTERFACE_NAME "org.kylin.kare"

//#define KYLIN_OS_KARE_DBUS_SERVICE_NAME "com.kylin.daq"
//#define KYLIN_OS_KARE_DBUS_PATH  "/com/kylin/daq"
//#define KYLIN_OS_KARE_DBUS_INTERFACE_NAME "com.kylin.daq.interface"

#include <QObject>
#include <QThread>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDBusReply>
#include <kprogressdialog.h>
#include "./package/PackageEnum.h"

class KylinInstalldbusCallThread  : public QThread
{
    Q_OBJECT
public:
    KylinInstalldbusCallThread(QString debPath,QString m_appName,QString m_application,QString m_applicationZh,QString m_appVersion);
    KylinInstalldbusCallThread(QString debPath,QString packageDistribution,PackageEnum::Type packageType);

private:
    QString           m_debPath;

    QString           m_appName;
    QString           m_application;
    QString           m_applicationZh;
    QString           m_appVersion;

    QString           m_osVersion;

    QDBusInterface   *m_pOsServiceInterface;
    QString           m_loginUserName;
    QString           m_loginUserId;
    PackageEnum::Type m_packageType;
    QString m_packageDistribution;
    QString m_installType;

    int SIGNAL1 = 0;


protected:
    void run();

signals:
    void kylinOsinstallStatus(bool status,QString msg);
    void kylinOsinstalldingStatus(int status);
    void kylinbatchOsinstalldingStatus(int status,QString installname);
    void kylinKareEnvInstallingStatus(int status);
    void kylinKareEnvInstallStatus(bool status,QString msg);


public slots:
    void osInstallOverSoftwareAptSlots(bool status,QString erro_string,QString erro_desc);
    void osInstallOverSoftwareSnapSlots(QString string, QMap<QString, QVariant> map);
    void osInstallOverstatus(int status,QString erro_string,QString erro_desc);
    QString getUserName();
    QString getUid();
    QString osVersion();
    bool copyFile(const QString &srcFile, const QString &destFile);
  //  void onCopyFinished(const QString &fileName, bool success);
    void installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    bool generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version);
    void getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor);
    bool isAndroidReady(const QString &username, const QString &uid);
    void generateIcon(const QString &pkgName);
   // void sendApkInfoToServer(const QString &os, const QString &cpuType, const QString &gpuVendor, const QString &pkgName, const QString &appName);
    QByteArray getInstalledAppListJsonStr();
  /*  QByteArray encrypt(const QByteArray &md5, const QByteArray &keyBase64);
    void evpError(void);*/

    void kareBuildStatusChangedSlot(QString env,int progress, int status , QString errorInfo);
    // void kareInstallFinishedSlot(QString debPath, int status);
    void kareInstallStatusSlot(QString debPath,int progress,int status,QString errorInfo);
    // void onAsyncCallFinished(QDBusPendingCallWatcher *watcher);
    void InstalldebStatusChangedSlot(int progress,QString status,QString currentDetails);
    void UpdateInstallFinishedSlot(bool status,QStringList upgradeGroup,QString errorString,QString errorDesc);
    void envBuildStatusChangedSlot(int status, QString info);
    void envBuildStatusFinishedSlot(bool status, QString info);
private:
    void notifyKareEnv();
    void installKareDeb();
    void installKareImage();
    void testV11KareInstall();

};

#endif // KYLININSTALLDBUSCALLTHREAD_H
