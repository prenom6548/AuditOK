#include <stdlib.h>
#include <QApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariant>
#include <QMessageBox>
#include <QDebug>
#include <QTranslator>
#include <QLibraryInfo>
#include "dbus_app_exit.h"

// 记录当前是否正在收集用户体验信息，防止保底退出机制误杀
// 如果正在收集用户体验信息，则为 true，此时保底退出机制失效
bool g_collecting = false;

enum class DBusAppStatus {
    kUnknow,
    kRunning,
    kNotRunning,
};

enum DBusAppStatus GetDBusAppStatus(const QString &uuid) {
    auto message = QDBusMessage::createMethodCall(DBUS_NAME,
                                                  DBUS_PATH,
                                                  DBUS_INTERFACE,
                                                  "IsAppRunning");
    message.setArguments(QList<QVariant>{uuid});
    auto reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qDebug() << "Call D-Bus check is running fail: " << reply.errorMessage();
        return DBusAppStatus::kUnknow;
    }
    auto args = reply.arguments();
    if (args.empty()) {
        qDebug() << "Call D-Bus check is running fail: dbus return vaule is empty";
        return DBusAppStatus::kUnknow;
    }
    if (args.front().toBool()) {
        return DBusAppStatus::kRunning;
    }

    return DBusAppStatus::kNotRunning;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    // 适配缩放和高分辨率
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    // 加载翻译文件
    QTranslator translator;
    if (translator.load(":/translations/app_launcher_" + QLocale::system().name() + ".qm")) {
        app.installTranslator(&translator);
    } else {
        qCritical() << "Load translation file fail";
    }
    QTranslator qt_translator;
    if (qt_translator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qt_translator);
    } else {
        qCritical() << "Load qt translation file fail";
    }
    QTranslator sdk_translator;
    const auto locale = QLocale::system().name();
    if (sdk_translator.load(":/translations/gui_" + locale + ".qm")) {
        app.installTranslator(&sdk_translator);
    } else {
        qCritical() << "Load sdk translation file fail";
    }

    QString uuid(argv[1]);
    auto dbus_app_status = GetDBusAppStatus(uuid);
    auto exec_message = QDBusMessage::createMethodCall(DBUS_NAME,
                                                       DBUS_PATH,
                                                       DBUS_INTERFACE,
                                                       "ExecApp");
    exec_message.setArguments(QList<QVariant>{uuid});
    auto exec_reply = QDBusConnection::sessionBus().call(exec_message);
    if (exec_reply.type() == QDBusMessage::MessageType::ErrorMessage) {
        qDebug() << "Call D-Bus launch app fail: " << exec_reply.errorMessage();
        return EXIT_FAILURE;
    }
    if (dbus_app_status == DBusAppStatus::kRunning) {
        qInfo() << "App already running";
        return EXIT_SUCCESS;
    }

    DBusAppExit dbus_app_exit(uuid);
    QDBusConnection::sessionBus().connect(DBUS_NAME,
                                          DBUS_PATH,
                                          DBUS_INTERFACE,
                                          "ExecAppFinished",
                                          &dbus_app_exit,
                                          SLOT(on_Exit(QString,QString)));

    // 保底方案，防止 D-Bus 启动的应用异常结束时无法接收到 D-Bus 信号，导致 app_launcher 无法退出
    auto *timer = new QTimer;
    timer->setInterval(1 * 10 * 1000);
    QObject::connect(timer, &QTimer::timeout, [uuid]() {
        if (g_collecting) {
            return;
        }

        auto status = GetDBusAppStatus(uuid);
        switch (status) {
            case DBusAppStatus::kRunning:
                return;
            default:
                break;
        }
        QTimer::singleShot(0, qApp, &QApplication::quit);
    });
    timer->start();

    return app.exec();
}
