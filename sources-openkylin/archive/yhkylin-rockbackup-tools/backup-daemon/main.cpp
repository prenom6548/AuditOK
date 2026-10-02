/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include <QCoreApplication>
#include <QTextCodec>
#include "../common/utils.h"
#include "../common/mydefine.h"
#include "backupmanager_adaptor.h"

int main(int argc, char *argv[])
{
    // 以防客户端传入的参数中的中文信息乱码
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    QCoreApplication a(argc, argv);
    a.setApplicationName("backup-daemon");
    a.setApplicationVersion("4.0.14");

    // 初始化日志
    Utils::initSysRootPath();
    qInstallMessageHandler(Utils::customMessageHandler);
    Utils::initSystemInfo();

    MyBackupManager backupManager;
    std::unique_ptr<ManagerAdaptor> managerAdaptor(new ManagerAdaptor(&backupManager));
    //new ManagerAdaptor(&backupManager);
    QDBusConnection conn = QDBusConnection::systemBus();

    if (!conn.registerService("com.kylin.backup")) {
        qDebug() << conn.lastError().message();
    }

    conn.registerObject("/", &backupManager);

    return a.exec();
}
