#include <QCoreApplication>
#include <QDebug>
#include "deviceinfodbusservice.h"
#include <libkylog.h>

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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);

    QCoreApplication a(argc, argv);
    DeviceInfoDbusService deviceInfoDbusService;
    if (!deviceInfoDbusService.rigisterService()) {
        return 1;
    }
    qDebug() << "DeviceInfoDbusService started";

    return a.exec();
}
