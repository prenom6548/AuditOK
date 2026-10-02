#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include "sessionclient.h"
#include <libkylog.h>
#include "include/monitorinfoinitializer.h"

#ifdef ICBC_ENVIRONMENT
#include "printer_backend.h"
#endif

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

    QApplication app(argc, argv);
    QString tranPath("/usr/share/kylin-os-manager/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-session-service", "_", tranPath)) {
        QCoreApplication::installTranslator(translator);
    } else {
        qDebug() << "ProblemFeedback load translation file fail !";
    }

    SessionClient sessionClient;

#ifdef ICBC_ENVIRONMENT
    doPrinterWork();
#endif

    MonitorInfoInitializer monitorInfo;
    return app.exec();
}