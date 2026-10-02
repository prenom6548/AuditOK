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
#ifndef TOOL_THREAD_H
#define TOOL_THREAD_H

#include <QThread>
#include <QVariant>
#include <QLibrary>
#include <QFile>
#include<sys/types.h>
#include<unistd.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QFileInfo>
#include <QDir>
#include "../src/common/utils/SystemInfoUtil.h"

#define LOCAL_PACKAGE_FLAG "local_pkg"
#define KAIMING_PACKAGE "kaiming"
#define LOCAL_ANDROID_PACKAGE_FLAG "local_Android_pkg"
#define ANDROID_SHARE_UNINSTALL_LIB "libkmre.so"
#define KARE_PACKAGE_FLAG "kare"

#define KYLINOS_KMRE_SERVICE_NAME "cn.kylinos.Kmre"
#define KYLINOS_KMRE_SERVICE_PATH "/cn/kylinos/Kmre"
#define KYLINOS_KMRE_INTERFACE_NAME "cn.kylinos.Kmre"

#define ORG_FREEDESKTOP_NOTIFICATIONS_NAME "org.freedesktop.Notifications"
#define ORG_FREEDESKTOP_NOTIFICATIONS_PATH "org/freedesktop/Notifications"
#define ORG_FREEDESKTOP_NOTIFICATIONS_INTERFACE_NAME "org.freedesktop.Notifications"

#define KYLINOS_KYDROID_SERVICE_NAME "cn.kylinos.Kydroid"
#define KYLINOS_KYDROID_SERVICE_PATH "/cn/kylinos/Kydroid"
#define KYLINOS_KYDROID_INTERFACE_NAME "cn.kylinos.Kydroid"

#define SYS_KMRE_BOOT_COMPLETED "sys.kmre.boot_completed"
#define SYS_KYDROID_BOOT_COMPLETED "sys.kydroid.boot_completed"

// #define KYLIN_OS_DBUS_SERVICE_NAME "com.kylin.systemupgrade"
// #define KYLIN_OS_DBUS_PATH  "/com/kylin/systemupgrade"
// #define KYLIN_OS_DBUS_INTERFACE_NAME "com.kylin.systemupgrade.interface"

#define KYLIN_OS_DBUS_SERVICE_NAME "com.kylin.systemupgrade"
#define KYLIN_OS_DBUS_PATH  "/com/kylin/systemupgrade"
#define KYLIN_OS_DBUS_INTERFACE_NAME "com.kylin.systemupgrade.interface"

class tool_thread : public QThread
{
   Q_OBJECT

public:
    tool_thread(QString runtime, QString app_name, QString version, QString zh_name, QString desktop_name);
    void uninstallAndroidInterface();
    bool getAndroidEnvWetherStart();
    void Slot_Message_Notification(int install,QString errorinfo);
    void deleteDesktopFile(QString desktopName);

protected:
   void run();

private:
   QString m_app_name;
   QString m_version;
   QString m_runtime;
   QString m_zh_name;
   QString libPath;
   QString m_desktop_name;

signals:
   void sig_uninstall_result(int);
   void sig_uninstall_status(int);
public slots:
   void slot_local_package_uninstall_result(bool status,QString error_string,QString error_desc);
   void slot_kaiming_package_uninstall_result(QString appName, int  type, int errorCode, QString error_desc);
   void slot_local_package();
   void slot_kare_package_uninstall_result(QString name,int status,QString errorInfo);

   //bool polkit();
};

#endif // TOOL_THREAD_H
