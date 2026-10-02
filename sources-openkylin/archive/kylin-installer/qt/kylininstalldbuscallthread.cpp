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
#include "kylininstalldbuscallthread.h"
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <dlfcn.h>
#include <qdir.h>
#include <qprocess.h>
#include <sys/stat.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <qtextstream.h>
#include <QSettings>
#include <QMessageBox>
#include "package/PackageUtils.h"
#include "../src/common/utils/SystemInfoUtil.h"
#include "../src/common/GlobalInfo.h"
#include "../src/common/utils/CustomBoxUtil.h"

KylinInstalldbusCallThread::KylinInstalldbusCallThread(QString debPath,QString m_appName,QString m_application,QString m_applicationZh,QString m_appVersion) : m_debPath(debPath),m_appName(m_appName),m_application(m_application),m_applicationZh(m_applicationZh),m_appVersion(m_appVersion)
{

}


KylinInstalldbusCallThread::KylinInstalldbusCallThread(QString debPath,QString packageDistribution,PackageEnum::Type packageType):m_debPath(debPath),m_packageType(packageType),m_packageDistribution(packageDistribution)
{

}


void KylinInstalldbusCallThread::run()
{
    qDebug()<<"m_debPath:"<<m_debPath;
    qDebug()<<"m_packageDistrionbution:"<<m_packageDistribution;
    qDebug()<<"m_packageType:"<<m_packageType;
    if(m_debPath.contains(".snap",Qt::CaseSensitive))
    {
        m_pOsServiceInterface = new QDBusInterface(KYLIN_OS_DBUS_SERVICE_NAME,
                                                   KYLIN_OS_DBUS_PATH,
                                                   KYLIN_OS_DBUS_INTERFACE_NAME,
                                                   QDBusConnection::systemBus());

        if (!m_pOsServiceInterface->isValid()) {
            qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
            emit kylinOsinstallStatus(false,"初始化dbus失败，dbus阻塞，请检查更新管理器服务是否正常");
            return ;
        }

        m_pOsServiceInterface->setTimeout(2147483647);

        QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                             KYLIN_OS_DBUS_INTERFACE_NAME, QString("KumSnapSignal"), \
                                             this, SLOT(osInstallOverSoftwareSnapSlots(QString, QMap<QString, QVariant>)));


        qDebug()<<"install_snap("<<m_debPath<<")";
        m_pOsServiceInterface->asyncCall("install_snap", m_debPath);
    }
    else if (m_debPath.contains(".apk",Qt::CaseSensitive)){

       QFileInfo fi(m_debPath);
       QString fileName = fi.fileName();
       m_loginUserName = getUserName();
       m_loginUserId = getUid();
       QString destPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/tmp").arg(m_loginUserId).arg(m_loginUserName);
       bool copy_status = copyFile(m_debPath,QString("%1/%2").arg(destPath).arg(fileName));
       if (copy_status) {
           installApp(fileName, m_appName, m_application, m_applicationZh, m_appVersion);
       }
       else {
            emit kylinOsinstallStatus(false,"拷贝apk文件到指定目录失败");
       }
    }
    else if (m_packageType == PackageEnum::Type::KARE_ENV && !m_packageDistribution.isEmpty())
    {
        qDebug() << "installing kare";
        if (!PackageUtils::isInstalledFromDpkg("kare"))
        {
            qDebug() << "install kare deb";
            installKareDeb();

        }
        else
        {
            qDebug() << "install kare image";
            installKareImage();
        }

    }
    else if (m_packageType == PackageEnum::Type::KARE) {

        m_pOsServiceInterface = new QDBusInterface(KYLIN_OS_KARE_DBUS_SERVICE_NAME,
                                                KYLIN_OS_KARE_DBUS_PATH,
                                                KYLIN_OS_KARE_DBUS_INTERFACE_NAME,
                                                QDBusConnection::systemBus());
        if (!m_pOsServiceInterface->isValid()) {
            qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
            emit kylinOsinstallStatus(false,"初始化dbus失败，dbus阻塞，请检查kare服务是否正常");
            return ;
        }

        QDBusConnection::systemBus().connect(KYLIN_OS_KARE_DBUS_SERVICE_NAME, KYLIN_OS_KARE_DBUS_PATH, \
                                            KYLIN_OS_KARE_DBUS_INTERFACE_NAME, QString("installStatusChanged"), \
                                            this, SLOT(kareInstallStatusSlot(QString,int, int, QString)));

        qDebug()<<"install("<<m_packageDistribution<<","<<m_debPath<<")";
        m_pOsServiceInterface->asyncCall("install",m_packageDistribution, m_debPath);

    }
    else{
        m_pOsServiceInterface = new QDBusInterface(KYLIN_OS_DBUS_SERVICE_NAME,
                                                   KYLIN_OS_DBUS_PATH,
                                                   KYLIN_OS_DBUS_INTERFACE_NAME,
                                                   QDBusConnection::systemBus());

        if (!m_pOsServiceInterface->isValid()) {
            qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
            emit kylinOsinstallStatus(false,"初始化dbus失败，dbus阻塞，请检查更新管理器服务是否正常");
            return ;
        }

        m_pOsServiceInterface->setTimeout(2147483647);

        QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                                     KYLIN_OS_DBUS_INTERFACE_NAME, QString("InstalldebFinished"), \
                                                     this, SLOT(osInstallOverSoftwareAptSlots(bool,QString,QString)));

        QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                                     KYLIN_OS_DBUS_INTERFACE_NAME, QString("InstalldebStatusChanged"), \
                                                     this, SLOT(osInstallOverstatus(int,QString,QString)));
        if (SystemInfoUtil::isOsTree)
        {
            QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                     KYLIN_OS_DBUS_INTERFACE_NAME, QString("EnvBuildStatusChanged"), \
                                     this, SLOT(envBuildStatusChangedSlot(int,QString)));

            QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                     KYLIN_OS_DBUS_INTERFACE_NAME, QString("EnvBuildStatusFinished"), \
                                     this, SLOT(envBuildStatusFinishedSlot(bool,QString)));

        }

        QString lang = getenv("LANG");
        qDebug() << "InstallDebFile(" << "kylin-installer" << "," << m_debPath << ",true,true," << lang << ")";

        if ( GlobalInfo::DebugFlag == false) {
            QDBusPendingReply<int, QString> reply = m_pOsServiceInterface->call(
                                                                "InstallDebFile",
                                                                "kylin-installer",
                                                                m_debPath,
                                                                true,
                                                                true,
                                                                lang );
            if (reply.isError()) {
                qDebug() << "[DBus] InstallDebFile call failed:"
                        << reply.error().name()
                        << reply.error().message();
            } else {
                int ret = reply.argumentAt<0>();
                QString errMsg = reply.argumentAt<1>();

                if (ret != 0) {
                    qDebug() << "[DBus] InstallDebFile returned error:" << ret << errMsg;
                    emit kylinOsinstallStatus(false,tr("Other applications are installing software packages, please try again later"));
                    return;
                }
            }

        } else {
            this->testV11KareInstall();
        }
    }
}

/*返回安装的进度值*/
void KylinInstalldbusCallThread::osInstallOverstatus(int status,QString erro_string,QString erro_desc)
{
    qDebug()<<"osInstallOverstatus("<<status<<","<<erro_string<<","<<erro_desc<<")";
     emit kylinOsinstalldingStatus(status);
     emit kylinbatchOsinstalldingStatus(status,m_debPath);
}

/* 安装完成信号，根据参数确认安装状态 */

void KylinInstalldbusCallThread::osInstallOverSoftwareAptSlots(bool status,QString erro_string,QString erro_desc)
{
    qDebug()<<"osInstallOverSoftwareAptSlots("<<status<<","<<erro_string<<","<<erro_desc<<")";
    QString map_string = erro_string + erro_desc;

    if (status == true) {
        emit kylinOsinstallStatus(true,map_string);
    } else {
        emit kylinOsinstallStatus(false,map_string);
    }

    return;
}

void KylinInstalldbusCallThread::osInstallOverSoftwareSnapSlots(QString string, QMap<QString, QVariant> map)
{
    qDebug() << "osInstallOverSoftwareSnapSlots("<<string<<","<<map<<")";

    QVariant map_test =  map["error_message"];
    QString map_string = map_test.toString();
    if (string == nullptr) {
        emit kylinOsinstallStatus(false,map_string);
        return;
    }
    if (string == "snap_finish") {
        emit kylinOsinstallStatus(true,map_string);
    } else {
        emit kylinOsinstallStatus(false,map_string);
    }
    return;
}

/********getusername******/
QString KylinInstalldbusCallThread::getUserName()
{
    std::string user_name;

    struct passwd *pwd = NULL;
    pwd = getpwuid(getuid());
    if (pwd && pwd->pw_name) {
        char* _user = pwd->pw_name;
        struct passwd *q = NULL;
        q = getpwnam(_user);
        if (q && (q->pw_uid != getuid())) {
            fprintf(stderr, "User name doesn't match uid.\n");
            return QString("");
        }

        return QString::fromStdString(std::string(pwd->pw_name));
    }
    else {
        user_name = std::getenv("USER");
        if (user_name.empty()) {
            user_name = std::getenv("USERNAME");
        }
        if (user_name.empty()) {
            char name[16];
            snprintf(name, sizeof(name), "%u", getuid());
            user_name = std::string(name);
        }
    }

    return QString::fromStdString(user_name);
}

/********getusrid********/
QString KylinInstalldbusCallThread::getUid()
{
    uid_t uid = getuid();
    return QString::number(uid);
}

QString KylinInstalldbusCallThread::osVersion()
{
    QString distribDescription;
    if (QFile::exists("/etc/lsb-release")) {
        QSettings lsbSetting("/etc/lsb-release", QSettings::IniFormat);
        distribDescription = lsbSetting.value("DISTRIB_DESCRIPTION").toString().replace("\"", "");
    }

    return distribDescription;
}

/********拷贝apk文件到指定目录下*****/
bool KylinInstalldbusCallThread::copyFile(const QString &srcFile, const QString &destFile)
{

    qDebug() << "Utils::copyFile:" << srcFile << destFile;
    QFile fp(destFile);
    if (fp.exists()) {
        if (!fp.remove()) {
            qCritical() << "Failed to remove:" << destFile;
            //return false;
        }
    }

    return QFile::copy(srcFile, destFile);
}

/*********安装apk文件**********/
void KylinInstalldbusCallThread::installApp(const QString &fileName, const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    if (fileName.isEmpty()) {
        emit kylinOsinstallStatus(false,"安装包包名为空，无法安装");
        return;
    }

    QString libPath = SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::INSTALL_LIB_PATH,"libkmre.so");
    if (!QFile::exists(libPath)) {
        emit kylinOsinstallStatus(false,"安装安卓包的库为空，请检查/usr/lib/libkmre.so库是否存在");
        return ;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        emit kylinOsinstallStatus(false,"module_handle 等于空");
        return;
    }
    bool (*install_app)(char *filename, char *appname, char *pkgname);
    install_app = (bool(*)(char *, char*, char *))dlsym(module_handle, "install_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        emit kylinOsinstallStatus(false,"module_handle获取的到值有误");
        return;
    }

    bool nRes = install_app(const_cast<char *>(fileName.toStdString().c_str()), const_cast<char *>("test"), pkgName.isEmpty() ? const_cast<char *>("test") : const_cast<char *>(pkgName.toStdString().c_str()));
    if (nRes) {
        updateDekstopAndIcon(pkgName, application, applicationZh, version);
        emit kylinOsinstallStatus(true,"sucess");
    }
    else {
        qDebug() << "BackendWorker::installApp failed: " << pkgName;
        emit kylinOsinstallStatus(false,"安装失败此apk文件并未适配操作系统");
    }
    dlclose(module_handle);
}
/*************設置dekstop文件************/
void KylinInstalldbusCallThread::updateDekstopAndIcon(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    QString cpuType = QString("");
    QString gpuVendor = QString("");
    getCpuAndGpuInfo(cpuType, gpuVendor);
    m_osVersion = osVersion();

    if (!pkgName.isEmpty() && !application.isEmpty() && !applicationZh.isEmpty()) {
        if (pkgName != "com.antutu.benchmark.full" && pkgName != "com.antutu.benchmark.full.lite") {
            generateDesktop(pkgName, application, applicationZh, version);
            generateIcon(pkgName);
        }
    }
    else {
        QByteArray array = getInstalledAppListJsonStr();
        QJsonParseError err;
        QJsonDocument document = QJsonDocument::fromJson(array, &err);//QJsonDocument document = QJsonDocument::fromJson(QString::fromStdString(std::string(running_info)).toLocal8Bit().data(), &err);
        if (err.error == QJsonParseError::NoError && !document.isNull() && !document.isEmpty() && document.isArray()) {
            QJsonArray jsonArray = document.array();
            foreach (QJsonValue val, jsonArray) {
                QJsonObject subObject = val.toObject();
                QString name = subObject.value("package_name").toString();
                if (name != "com.antutu.benchmark.full" && name != "com.antutu.benchmark.full.lite") {
                    //qDebug() << "Test App: " << subObject.value("app_name").toString() << subObject.value("package_name").toString() << subObject.value("version_name").toString();
                    bool b = generateDesktop(name, subObject.value("app_name").toString(), subObject.value("app_name").toString(), subObject.value("version_name").toString());
                    if (b) {
                        qDebug() << subObject.value("package_name").toString() << "installed!";
                    }
                    this->generateIcon(subObject.value("package_name").toString());
                }
            }
        }
    }
}

/*********設置dektop文件*****/
bool KylinInstalldbusCallThread::generateDesktop(const QString &pkgName, const QString &application, const QString &applicationZh, const QString &version)
{
    if (pkgName.isEmpty()) {
        return false;
    }

    const QString desktopsPath = QString("%1/.local/share/applications").arg(QDir::homePath());
    if (!QDir().exists(desktopsPath)) {
        QDir().mkdir(desktopsPath);
    }

    const QString destDesktopFile = QString("%1/.local/share/applications/%2.desktop").arg(QDir::homePath()).arg(pkgName);
    QFile desktopFp(destDesktopFile);
    if (desktopFp.exists()) {
        return false;
    }
    QFile file(destDesktopFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    if (version.isNull() || version.isEmpty()) {
        out << "[Desktop Entry]\n"
            "Terminal=false\n"
            "Type=Application\n"
            "StartupNotify=false\n"
            "Keywords=Android;App;Apk\n"
            "Categories=Android;App;Apk\n"
            "Name=" + application + "\n"
            "Name[zh_CN]=" + applicationZh + "\n"
            "Comment=" + application + "\n"
            "Comment[zh_CN]=" + applicationZh + "\n"
            "Exec=/usr/bin/startapp " + pkgName + "\n"
            "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".png"
            << endl;
    }
    else {
        out << "[Desktop Entry]\n"
            "Terminal=false\n"
            "Type=Application\n"
            "StartupNotify=false\n"
            "Keywords=Android;App;Apk\n"
            "Categories=Android;App;Apk\n"
            "Name=" + application + "\n"
            "Name[zh_CN]=" + applicationZh + "\n"
            "Comment=" + application + "\n"
            "Comment[zh_CN]=" + applicationZh + "\n"
            "Exec=/usr/bin/startapp " + pkgName + " " + version + "\n"
            "Icon=" + QDir::homePath() + "/.local/share/icons/" + pkgName + ".png"
            << endl;
    }
    out.flush();
    file.close();

    chmod(destDesktopFile.toStdString().c_str(), 0770);
    QProcess::startDetached(QString("/usr/bin/update-desktop-database %1").arg(desktopsPath));

    return true;
}
//设置icon
void KylinInstalldbusCallThread::generateIcon(const QString &pkgName)
{
    if (pkgName.isEmpty()) {
        return;
    }

    const QString iconsPath = QString("%1/.local/share/icons").arg(QDir::homePath());
    if (!QDir().exists(iconsPath)) {
        QDir().mkdir(iconsPath);
    }

    const QString linuxIconPath = QString("%1/%2.png").arg(iconsPath).arg(pkgName);
    const QString linuxSvgPath = QString("%1/%2.svg").arg(iconsPath).arg(pkgName);
    const QString containerIconPath = QString("/var/lib/kmre/kmre-%1-%2/data/local/icons/%3.png").arg(m_loginUserId).arg(m_loginUserName).arg(pkgName);
    QFile cfp(containerIconPath);
    QFile fp(linuxIconPath);
    if (cfp.exists() && !fp.exists()) {
        copyFile(containerIconPath, linuxIconPath);
        QProcess::startDetached(QString("/usr/sbin/update-icon-caches %1").arg(iconsPath));
    }
}

void KylinInstalldbusCallThread::getCpuAndGpuInfo(QString &cpuType, QString &gpuVendor)
{
    m_loginUserName = getUserName();
    m_loginUserId = getUid();
    bool androidReady = isAndroidReady(m_loginUserName, m_loginUserId);
    if (androidReady) {
        QDBusInterface mSessionBusInterface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
        QString displayInfo;
        QDBusMessage response = mSessionBusInterface.call("getDisplayInformation");
        if (response.type() == QDBusMessage::ReplyMessage) {
            displayInfo = response.arguments().takeFirst().toString();
            if (!displayInfo.isEmpty()) {
                QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
                if (!jsonDocument.isNull()) {
                    QJsonObject jsonObject = jsonDocument.object();
                    if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                        if (jsonObject.contains("cpu_type")) {
                            cpuType = jsonObject.value("cpu_type").toString();
                        }
                        if (jsonObject.contains("gpu_vendor")) {
                            gpuVendor = jsonObject.value("gpu_vendor").toString();
                        }
                    }
                }
            }
        }
    }

    if (gpuVendor.isNull() || gpuVendor.isEmpty()) {
        gpuVendor = "unknown";
    }
}

bool KylinInstalldbusCallThread::isAndroidReady(const QString &username, const QString &uid)
{
    QString value;
    QDBusInterface *interface;
    interface = new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus());
    QDBusMessage response = interface->call("GetPropOfContainer", username, uid.toInt(), "sys.kmre.boot_completed");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }
    delete interface;
    if (value == "1") {
        return true;
    }
    else {
        return false;
    }
}

QByteArray KylinInstalldbusCallThread::getInstalledAppListJsonStr()
{

    QString libPath = SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::INSTALL_LIB_PATH,"libkmre.so");
    if (!QFile::exists(libPath)) {
        return QString("[]").toUtf8();
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return QString("[]").toUtf8();
    }

    char *(*get_installed_applist)();
    get_installed_applist = (char *(*)())dlsym(module_handle, "get_installed_applist");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return QString("[]").toUtf8();
    }

    char *list_info = NULL;
    list_info = get_installed_applist();
    if (list_info) {
        std::string runningInfo = std::string(list_info);
        QByteArray byteArray(list_info, runningInfo.length());
        dlclose(module_handle);
        return byteArray;
    }

    dlclose(module_handle);
    return QString("[]").toUtf8();
}

void KylinInstalldbusCallThread::kareBuildStatusChangedSlot(QString env,int progress, int status , QString errorInfo)
{
    qDebug()<<"kareBuildStatusChangedSlot("<<env<<","<<progress<<","<<status<<","<<errorInfo<<")";
    if (status == 1)
    {
        emit kylinKareEnvInstallingStatus(50+progress/2);
    }
    else if (status == 0)
    {
        emit kylinKareEnvInstallStatus(true,errorInfo);
    }
    else if (status == -1) 
    {
        notifyKareEnv();
        emit kylinKareEnvInstallStatus(false,errorInfo);
    }

}

void KylinInstalldbusCallThread::kareInstallStatusSlot(QString debPath,int progress,int status,QString errorInfo)
{
    qDebug()<<"kareInstallStatusSlot("<<debPath<<","<<progress<<","<<status<<","<<errorInfo<<")";
    if (status == 1)
    {
        emit kylinOsinstalldingStatus(50+progress/2);
        emit kylinbatchOsinstalldingStatus(50+progress/2,m_debPath);
    }
    else if (status == 0)
    {
        emit kylinOsinstallStatus(true,errorInfo);
    }
    else
    {
        emit kylinOsinstallStatus(false,errorInfo);
    }

}

void KylinInstalldbusCallThread::notifyKareEnv()
{

    QString dbus_service = "org.freedesktop.Notifications";
    QString dbus_path = "/org/freedesktop/Notifications";
    QString dbus_interface = "org.freedesktop.Notifications";
    QString notify = "Notify";
    QList<QVariant> args;
    QStringList action;
    QMap<QString,QVariant> hint;
    QString name = QObject::tr("kylin-installer");
    QString title;
    QString string ;

    // title = QObject::tr("System Notification");
    string = QObject::tr("Description Failed to initialize the KARE compatible environment");

    uint32_t replaceid = 0;
    QString icon = "kylin-installer";
    args << name
         << replaceid
         << icon
         << title
         << string
         << action
         << hint
         << -1;
    QDBusInterface *iface = new QDBusInterface(dbus_service, dbus_path, dbus_interface, QDBusConnection::sessionBus());
    iface->asyncCallWithArgumentList(notify,args);

    return;
}


void KylinInstalldbusCallThread::InstalldebStatusChangedSlot(int progress,QString status,QString currentDetails)
{
    qDebug()<<"InstalldebStatusChangedSlot("<<progress<<","<<status<<","<<currentDetails<<")";
    emit kylinKareEnvInstallingStatus(progress/2);

}

void KylinInstalldbusCallThread::UpdateInstallFinishedSlot(bool status,QStringList upgradeGroup,QString errorString,QString errorDesc)
{
    qDebug()<<"UpdateInstallFinishedSlot("<<status<<","<<upgradeGroup<<","<<errorString<<","<<errorDesc<<")";
    QString errorInfo = errorString + errorDesc;
    if (status)
    {
        this->installKareImage();
    }
    else
    {
        emit kylinKareEnvInstallStatus(false,errorInfo);
        notifyKareEnv();
    }
}

void KylinInstalldbusCallThread::installKareDeb()
{
    m_pOsServiceInterface = new QDBusInterface(KYLIN_OS_DBUS_SERVICE_NAME,
                                            KYLIN_OS_DBUS_PATH,
                                            KYLIN_OS_DBUS_INTERFACE_NAME,
                                            QDBusConnection::systemBus());

    if (!m_pOsServiceInterface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        emit kylinKareEnvInstallStatus(false,"初始化dbus失败，dbus阻塞，请检查更新管理器服务是否正常");
        return ;
    }

    m_pOsServiceInterface->setTimeout(2147483647);

    QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                                KYLIN_OS_DBUS_INTERFACE_NAME, QString("InstalldebStatusChanged"), \
                                                this, SLOT(InstalldebStatusChangedSlot(int,QString,QString)));

    QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME, KYLIN_OS_DBUS_PATH, \
                                                KYLIN_OS_DBUS_INTERFACE_NAME, QString("UpdateInstallFinished"), \
                                                this, SLOT(UpdateInstallFinishedSlot(bool,QStringList,QString,QString)));

    QVariantList appList;
    appList.append("kare");
    qDebug()<<"install kare deb";
    m_pOsServiceInterface->asyncCall("InstallPackages",appList);

}

void KylinInstalldbusCallThread::installKareImage()
{

    m_pOsServiceInterface = new QDBusInterface(KYLIN_OS_KARE_DBUS_SERVICE_NAME,
                                                KYLIN_OS_KARE_DBUS_PATH,
                                                KYLIN_OS_KARE_DBUS_INTERFACE_NAME,
                                                QDBusConnection::systemBus());
    if (!m_pOsServiceInterface->isValid()) {
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        emit kylinKareEnvInstallStatus(true,"初始化dbus失败，dbus阻塞，请检查kare服务是否正常");
        return ;
    }
    QDBusConnection::systemBus().connect(KYLIN_OS_KARE_DBUS_SERVICE_NAME, KYLIN_OS_KARE_DBUS_PATH, \
                                        KYLIN_OS_KARE_DBUS_INTERFACE_NAME, QString("buildStatusChanged"), \
                                        this, SLOT(kareBuildStatusChangedSlot(QString,int, int, QString)));
    qDebug()<<"buildBaseEnv("<<m_packageDistribution<<")";
    m_pOsServiceInterface->asyncCall("buildBaseEnv", m_packageDistribution);

}


void KylinInstalldbusCallThread::envBuildStatusChangedSlot(int status, QString info)
{
    qDebug()<<"envBuildStatusChangedSlot("<<status<<","<<info<<")";
    emit kylinKareEnvInstallingStatus(status);

}

void KylinInstalldbusCallThread::envBuildStatusFinishedSlot(bool status, QString info)
{
    qDebug()<<"envBuildStatusFinishedSlot("<<status<<", "<<info<<")";
    if (status)
    {
        emit kylinKareEnvInstallStatus(true,info);
    }
    else
    {
        notifyKareEnv();
        emit kylinKareEnvInstallStatus(false,info);
    }
}

void KylinInstalldbusCallThread::testV11KareInstall()
{
    for (int index=0;index <= 100;index+=10)
    {
        sleep(1);
        envBuildStatusChangedSlot(index,"test");
    }
    envBuildStatusFinishedSlot(true,"fes");
    for (int index=0;index <= 100;index+=10)
    {
        sleep(1);
        osInstallOverstatus(index,"test2","test3");
    }
    osInstallOverSoftwareAptSlots(true,"test4","test6");
}
