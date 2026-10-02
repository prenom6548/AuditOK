#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <libkylog.h>
#include "kom-utils.h"
#include "dbus.h"
#include "disk_monitor.h"

#define SECURITY_LIB "/usr/lib/kysdk/kysdk-security/libkyprocesssec.so"

typedef int (*KdkProcessEnableNetworkingFunc)(const char *filepath);

static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *func = context.function ? context.function : "";

    switch (type) {
        case QtDebugMsg:
            klog_debug("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtInfoMsg:
            klog_info("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtWarningMsg:
            klog_warning("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtCriticalMsg:
            klog_err("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtFatalMsg:
            klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(logOutput);

    QCoreApplication app(argc, argv);

    // 添加安全中心网络控制白名单
    kom::KomUtils::setNetworkWhiteListFromPackage(kom::KomUtils::Service);

    dbus::DbusManager dbusManager;

    // 创建线程，启动磁盘监控
    monitor::DiskMonitor disk_monitor;
    QThread disk_monitor_thread;
    disk_monitor.moveToThread(&disk_monitor_thread);
    QObject::connect(&disk_monitor_thread, &QThread::started, &disk_monitor, &monitor::DiskMonitor::on_Run);
    disk_monitor_thread.start();

    return app.exec();
}
