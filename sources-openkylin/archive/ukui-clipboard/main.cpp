#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QKeyEvent>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QQuickWindow>
#include <QTranslator>
#include <QDebug>
#include <QGuiApplication>

#include <ukui-log4qt.h>

#include "qtsingleapplication.h"
#include "clipboardbackend.h"
#include "windowsettings.h"

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-clipboard");
    qDebug()<<"ukui-clipboard start!";

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QtSingleApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption showWorkSpaceOption("show-menu","show contentMenu");
    parser.addOption(showWorkSpaceOption);
    parser.process(app);
    if (app.isRunning())
    {
        if (parser.isSet("show-menu")) {
            app.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : app.applicationFilePath());
        }
        return EXIT_SUCCESS;
    }

    QTranslator translator;
    const QString locale = QLocale::system().name();
    const QString path = "/usr/share/ukui/translations/ukui-clipboard/ukui-clipboard_" + locale;
    if (translator.load(path)) {
        app.installTranslator(&translator);
    } else {
        qWarning()<< "Load translation file failed";
    }

    QQmlApplicationEngine engine;
    //注册了ContentMenu这个类，在qml文件中import这个类则可互相调用
    qmlRegisterUncreatableType<ContentMenu>(
        "ContentMenu", 1, 0, "ContentType",
        "Not be instantiated in QML, we should only pass the enum types to QML "
        "!");
    ClipboardBackend backend;
    QObject::connect(&app, &QtSingleApplication::messageReceived, [&backend](){
        backend.open();
    });

    //将backend对象注册到元对象系统中，即将backend注册到QML中
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.addImageProvider(QLatin1String("inMemoryImages"), backend.getImageProvider());
    const QUrl url(QStringLiteral("qrc:/FunctionMenuWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    const QUrl url1(QStringLiteral("qrc:/ClipboardWindow.qml"));
    //engine.load(url);
    //engine.load(url1);
    QObject::connect(&backend, &ClipboardBackend::actionTriggered, [&engine, url, url1]{
        engine.load(url);
        engine.load(url1);
    });

    //WindowSettings settings;

    return app.exec();
}
