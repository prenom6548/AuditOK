/*
 *  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
 *  yhkylin-systembackup-tools is licensed under GPL-2+
 *  See LICENSE file for more details.
 */

#include <QCoreApplication>
#include <QTextCodec>
#include "../common/utils.h"
#include "../common/mydefine.h"
#include "backupmanager_adaptor.h"
#include <memory>
#include "calcworker.h"

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

    CalcWorker *calcworker = new CalcWorker();
    // 计算备份数据大小和剩余空间大小线程
    MyThread calcWorkerThread;

    // 后台daemon程序起来的时候，就开始另起一个线程计算备份数据大小，用于系统备份环境检测阶段
    calcworker->moveToThread(&calcWorkerThread);
    QObject::connect(&calcWorkerThread, &MyThread::started, calcworker, &CalcWorker::doWork);
    QObject::connect(&calcWorkerThread, &MyThread::finished, calcworker, &CalcWorker::deleteLater);

    calcWorkerThread.start();

    MyBackupManager backupManager;
    //new ManagerAdaptor(&backupManager);
    std::unique_ptr<ManagerAdaptor> adaptor(new ManagerAdaptor(&backupManager));
    QDBusConnection conn = QDBusConnection::systemBus();

    if (!conn.registerService("com.kylin.systembackup")) {
        qDebug() << conn.lastError().message();
    }

    conn.registerObject("/update", &backupManager);

    return a.exec();
}
