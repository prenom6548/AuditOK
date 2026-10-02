#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QLabel>
#include <libkylog.h>
#include "sessionclient.h"
#include "include/monitorinfoinitializer.h"

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

    // 适配高分辨率和缩放
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6 , 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    // 设置主界面关闭时进程不退出
    app.setQuitOnLastWindowClosed(false);

    // 加载翻译文件
    QString tranPath("/usr/share/kylin-os-manager/translations");
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), "kylin-os-manager-session-service", "_", tranPath)) {
        QCoreApplication::installTranslator(translator);
    } else {
        qWarning() << "Load translation file fail";
    }
    // 加载 sdk 翻译文件
    auto locale = QLocale::system().name();
    QTranslator sdk_translator;
    if (sdk_translator.load(":/translations/gui_" + locale + ".qm")) {
        app.installTranslator(&sdk_translator);
    } else {
        qWarning() << "Load sdk translation file fail";
    }

    SessionClient sessionClient;

    MonitorInfoInitializer monitorInfo;

    return app.exec();
}
