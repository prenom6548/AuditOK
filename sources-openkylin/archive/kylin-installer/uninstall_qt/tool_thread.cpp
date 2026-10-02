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
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusReply>
#include <QString>
#include <QMap>
#include "tool_thread.h"
#include "single.h"
#include "lib_fun.h"
#include <QCoreApplication>
#include <dlfcn.h>


tool_thread::tool_thread(QString runtime, QString app_name, QString version, QString zh_name, QString desktop_name)
{
    this->m_runtime = runtime;
    this->m_app_name = app_name;
    this->m_version = version;
    this->m_zh_name = zh_name;
    this->m_desktop_name = desktop_name;
    /*测试代码*/
    qDebug() << "tool_thread::tool_thread::init()";
    qDebug() << "tool_thread::tool_thread::runtime="<< m_runtime;
    qDebug() << "tool_thread::tool_thread::app_name="<< m_app_name;
    qDebug() << "tool_thread::tool_thread::version="<< m_version;
    qDebug() << "tool_thread::tool_thread::zh_name="<<zh_name;
    qDebug() << "tool_thread::tool_thread::desktop_name="<< desktop_name;
}

bool tool_thread::getAndroidEnvWetherStart()
{
    QString userName = QString(getenv("LOGNAME"));
    int usetId = getuid();
    QString confPath = SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::KMRE_CONF_PATH,"kmre.conf");
    QFile file(confPath);
    if (file.exists()) {
        QDBusInterface *p_kmreServiceInterface = new QDBusInterface(KYLINOS_KMRE_SERVICE_NAME,
                                                                    KYLINOS_KMRE_SERVICE_PATH,
                                                                    KYLINOS_KMRE_INTERFACE_NAME,
                                                                    QDBusConnection::systemBus());

        if (!p_kmreServiceInterface->isValid()) {
            qWarning() << "dbus接口设置失败";
            delete p_kmreServiceInterface;
            p_kmreServiceInterface = nullptr;
            return false;
        }

        QDBusReply<QString> reply = p_kmreServiceInterface->call("GetPropOfContainer",
                                                                 userName,
                                                                 usetId,
                                                                 SYS_KMRE_BOOT_COMPLETED);
        qDebug() << userName << usetId << "sys.kmre.boot.completed" ;
        delete p_kmreServiceInterface;
        p_kmreServiceInterface = nullptr;
        if (reply.value() == "1") {
            return true;
        }
    } else {
        QDBusInterface *p_kyDroidServiceInterface = new QDBusInterface(KYLINOS_KMRE_SERVICE_NAME,
                                                                       KYLINOS_KMRE_SERVICE_PATH,
                                                                       KYLINOS_KMRE_INTERFACE_NAME,
                                                                       QDBusConnection::systemBus());
        if (!p_kyDroidServiceInterface->isValid()) {
            qWarning() << "dbus接口设置失败";
            delete p_kyDroidServiceInterface;
            p_kyDroidServiceInterface = nullptr;
            return false;
        }
        QDBusReply<QString> reply = p_kyDroidServiceInterface->call("GetPropOfContainer",
                                                                    userName,
                                                                    usetId,
                                                                    SYS_KYDROID_BOOT_COMPLETED);
        delete p_kyDroidServiceInterface;
        p_kyDroidServiceInterface = nullptr;
        if (reply.value() == "1") {
            return true;
        }
    }
    return false;
}



void tool_thread::uninstallAndroidInterface()
{

    QString libPath = SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::INSTALL_LIB_PATH,"libkmre.so");
    if (!QFile::exists(libPath)) {
        Slot_Message_Notification(-1,"不存在安卓库，卸载失败");
        emit sig_uninstall_result(-1);
        return ;
    }
    this->libPath = libPath;
    QThread::sleep(1);
    qDebug() << libPath;

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
    bool (*uninstall_app)(char *appname);
    uninstall_app = (bool(*)(char *))dlsym(module_handle, "uninstall_app");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
    //pkgname:包名，如 com.tencent.mm
    bool nRes = uninstall_app(const_cast<char *>(m_app_name.toStdString().c_str()));
    if (nRes) {
        QThread::sleep(1);
        dlclose(module_handle);

        QDBusInterface *server_uninstall = NULL;
        server_uninstall = new QDBusInterface("com.kylin.softwarestore.server",
                                              "/com/kylin/softwarestore/server", \
                                              "com.kylin.softwarestore.server",
                                              QDBusConnection::sessionBus());
        if (!server_uninstall->isValid())
        {
            qInfo() << "DBus service not  start....";
        }
        else
        {
            server_uninstall->asyncCall("acceptUinstallApk",this->m_app_name);

        }
        emit sig_uninstall_result(1);
    }
    else {
        dlclose(module_handle);
        Slot_Message_Notification(-1,"安卓库报错，卸载失败");
        emit sig_uninstall_result(-1);
    }
}

void tool_thread::run()
{
    QString username = QString(getenv("LOGNAME"));
    /* 是否成功卸载标志 ， -1: 失败 0：成功 */
    int i_result_flag = -2;
    QDBusInterface *server_fun = NULL;
    int i_ret = -1;

    /* 判断runtime与app_name是否为空 */
    if (this->m_runtime.isEmpty() || this->m_app_name.isEmpty()) {
        qDebug() << "runtime为空";
        i_result_flag = -1;
        goto FLAG;
    }
    /* 当为本地包时 */
    if (this->m_runtime == LOCAL_PACKAGE_FLAG  || this->m_runtime == KAIMING_PACKAGE) {
        qDebug() << "Info : this is local pkg  or kaiming package";
        QDBusReply<bool> reply;
        /*创建接口映射*/
        server_fun = new QDBusInterface(KYLIN_OS_DBUS_SERVICE_NAME,
                                        KYLIN_OS_DBUS_PATH, \
                                        KYLIN_OS_DBUS_INTERFACE_NAME,
                                        QDBusConnection::systemBus());
        if (!server_fun->isValid()) {
            qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
            i_result_flag = -1;
            goto FLAG;
        }
        /*绑定本地卸载接口信号*/
        QDBusConnection::systemBus().connect(KYLIN_OS_DBUS_SERVICE_NAME,
                                             KYLIN_OS_DBUS_PATH, \
                                             KYLIN_OS_DBUS_INTERFACE_NAME,
                                             QString("PurgePackagesFinished"), \
                                             this,
                                             SLOT(slot_local_package_uninstall_result(bool,QString,QString)));

        QString lang = getenv("LANG");
        QStringList applist;
        applist.append(this->m_app_name);
        qDebug()<<"tool_tread::run::PurgePackages("<<applist<<","<<username<<","<<lang<<")";

        QDBusPendingReply<int, QString> purgeReply = server_fun->call(
                                                            "PurgePackages",
                                                            applist,
                                                            username,
                                                            lang );
        if (purgeReply.isError()) {
            qDebug() << "[DBus] PurgePackages call failed:"
                    << purgeReply.error().name()
                    << purgeReply.error().message();
        } else {
            int ret = purgeReply.argumentAt<0>();
            QString errMsg = purgeReply.argumentAt<1>();

            if (ret != 0) {
                qDebug() << "[DBus] PurgePackages returned error:" << ret << errMsg;
                Slot_Message_Notification(-1, tr("Other applications are installing software packages, please try again later"));
                emit sig_uninstall_result(-1);
                return;
            }
        }

    } else if (this->m_runtime == LOCAL_ANDROID_PACKAGE_FLAG) {
        uninstallAndroidInterface();
    }else {
        //kare删除对应的desktop文件
        if (!m_runtime.compare("kare")){

            QString cmd = "pkexec --user root rm " + m_desktop_name + " -f";
            qDebug() << "file <<<<<" << cmd;
            system(cmd.toLocal8Bit().data());

            i_result_flag = 0;
            goto FLAG;
        }
        /*为兼容环境中的包*/
        /*创建接口映射*/
        qDebug() << "Info : this is runtim  e environment pkg";
        QDBusReply<int> reply;
        server_fun = new QDBusInterface("com.kylin.runtime" , "/com/kylin/runtime" , "com.kylin.runtime.interface" , QDBusConnection::systemBus());
        if (!server_fun->isValid()) {
            qDebug() << "创建dbus接口映射失败：" << QDBusConnection::systemBus().lastError().message();
            i_result_flag = -1;
            goto FLAG;
        }

        printf("Info : start uninstall compatible environment pkg...\n");


        printf("Info : lock uninstall lock success\n");

        /*调用服务端卸载函数*/
        server_fun->asyncCall("kre_uninstall_app" , this->m_runtime , this->m_app_name);

        /*判断是否卸载成功*/
        server_fun->asyncCall("kre_install_status" , this->m_runtime , this->m_app_name);
        if (!reply) {
            /*卸载成功*/
            i_result_flag = 0;
            printf("Info : uninstall compatible environment pkg success\n");
            /*删除json配置文件*/
            server_fun->asyncCall("delete_json_file" , this->m_runtime , this->m_app_name);
        } else {
            /*卸载失败*/
            i_result_flag = -1;
            printf("Error : uninstall compatibel enviroment pkg fail\n");
        }

        /*unlock file*/
        //        i_ret =  unlock_uninstall_file();
        //        if (i_ret == -1) {
        //           printf("Error : unlock uninstall lock fail\n");
        //        }
        //        printf("Info : unlock uninstall lock success\n");
    }

    delete server_fun;
    server_fun = nullptr;

FLAG:
    if (i_result_flag == -1) {
        emit sig_uninstall_result(-1);
    } else if(i_result_flag == 0) {
        emit sig_uninstall_result(0);
    } else {
        ;
    }
    return;
}

void tool_thread::slot_local_package_uninstall_result(bool status,QString error_string,QString error_desc)
{
    int i_ret = -1;
    qDebug()<<"tool_tread::slot_local_package_uninstall_result("<<status<<","<<error_string<<","<<error_desc<<")";
    if (status == true) {
        /*刪除桌面快捷方式*/
        deleteDesktopFile(m_desktop_name);

        // 当卸载成功后，删除desktop文件
        QString cmd = "pkexec --user root rm " + m_desktop_name + " -f";
        qDebug() << "file <<<<<" << cmd;
        system(cmd.toLocal8Bit().data());
        // 发送卸载成功信号
        emit sig_uninstall_result(0);
    }
    else {
        QString str = error_string; 
        if (error_desc != "") {
            str = error_string + "," + error_desc;
        }
        Slot_Message_Notification(-1,str);
        emit sig_uninstall_result(-1);
    }


    /*unlock file*/
    //i_ret =  unlock_uninstall_file();
    //     if (i_ret == -1) {
    //         printf("Error : unlock uninstall file fail\n");
    //     }

    //     printf("Info : unlock uninstall lock success\n");

    //     if (unlock_extra_update_lock()) {
    //        printf("Error : unlock extra update lock fail\n");
    //     }

    //     printf("Info : unlock extra update lock success\n");

    return;
}


void tool_thread::slot_kaiming_package_uninstall_result(QString appName, int type, int errorCode, QString error_desc)
{
    int i_ret = -1;
    if (type == 2) {
        if (errorCode == 0) {
            emit sig_uninstall_result(0);
        } else {
            QString str = error_desc;
            Slot_Message_Notification(-1,str);
            emit sig_uninstall_result(-1);
        }
    }
    return;
}


void tool_thread::slot_local_package()
{
    emit sig_uninstall_status(0);
}

void tool_thread::Slot_Message_Notification(int result,QString errorinfo)
{

    QString dbus_service = "org.freedesktop.Notifications";
    QString dbus_path = "/org/freedesktop/Notifications";
    QString dbus_interface = "org.freedesktop.Notifications";
    QString notify = "Notify";
    QList<QVariant> args;
    QStringList action;
    QMap<QString,QVariant> hint;
    QString name = QObject::tr("kylin-uninstaller");
    QString title;
    QString string ;

    if (result == 0){
        title = QObject::tr("Software uninstallation");
        string = QObject::tr("Software uninstalled successfully");
    }
    else{
        title = QObject::tr("Software uninstallation failed");
        string = errorinfo;
    }

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


void tool_thread::deleteDesktopFile(QString desktopName)
{
    QFileInfo SrcFile(desktopName);
    QString fileName = SrcFile.fileName();

    QStringList desktopDirs = {
        QDir::homePath() + "/Desktop",
        QDir::homePath() + "/桌面",
        QDir::homePath() + "/ཅོག་ངོས།"
    };

    for (const QString &dir : desktopDirs) {
        QFile file(dir + "/" + fileName);
        if (file.exists()) {
            qDebug() << "Find desktop file, removing:" << file.fileName();
            if (!file.remove())
                qWarning() << "Failed to remove" << file.fileName();
        }
    }
}

void tool_thread::slot_kare_package_uninstall_result(QString name,int status,QString errorInfo)
{
    int i_ret = -1;
    if (status == 0) {
        QString cmd = "pkexec --user root rm " + m_desktop_name + " -f";
        qDebug() << "file <<<<<" << cmd;
        system(cmd.toLocal8Bit().data());
        /*刪除桌面快捷方式*/
        deleteDesktopFile(m_desktop_name);

        emit sig_uninstall_result(0);
        qWarning()<<"slot_kare_package_uninstall_result("<<name<<","<<status<<","<<errorInfo<<");";
    }
    else if (status == -1) {
//        QString str = error_string + "," + error_desc;
        qWarning()<<"slot_kare_package_uninstall_result("<<name<<","<<status<<","<<errorInfo<<");";
        Slot_Message_Notification(-1,QString("kare uninstall %s fail").arg(name));
        emit sig_uninstall_result(-1);
    }

}
