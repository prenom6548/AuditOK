#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>

#include "qtsingleapplication.h"
#include <ukui-log4qt.h>
#include "clipboardtray.h"

int  main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-clipboard-tray");
    qDebug()<<"ukui-clipboard-tray start!";

    //跟随系统缩放
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QtSingleApplication app(argc,argv);

    QTranslator translator;
    if (!translator.load(QLocale::system(), QLatin1String("ukui-clipboard"),QLatin1String("_"), QLatin1String(":/"))){
        // 如果指定语言的翻译文件未找到，尝试加载默认的翻译文件
        if (!translator.load(QString("ukui-clipboard"))) {
            qWarning() << "Failed to load translation files!";
        }
    }
    app.installTranslator(&translator);

    if (app.isRunning())
    {
        qDebug()<<"ukui-clipboard-tray is running";
        return EXIT_SUCCESS;
    }

    //此判断需要放在application初始化后，不然会报莫名错误
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning()<<">>>>>>  系统不支持托盘~";
    }

    ClipboardTray tray;
    QQmlApplicationEngine engine;

//    qmlRegisterType<clipboardData>("MyComponents", 1, 0, "clipboardData");
//    qmlRegisterType<Clipboardmodel>("MyComponents", 1, 0, "Clipboardmodel");

    // 将C++对象注册到QML上下文中
    engine.rootContext()->setContextProperty("tray", &tray);

    // 加载QML文件
    engine.load(QUrl(QStringLiteral("qrc:/source/ClipboardMainWindow.qml")));
    if (engine.rootObjects().isEmpty()) {
        qWarning()<<"load ClipboardMainWindow.qml error";
        return -1;
    }

    QObject *rootObject = engine.rootObjects().first();
    if (QQuickWindow *window = qobject_cast<QQuickWindow*>(rootObject)) {
        window->setIcon(QIcon(":/source/data/kylin-clipboard.svg"));
        // 移除最大化按钮
        //window->setFlags(window->flags() & ~Qt::WindowMaximizeButtonHint);
        //window->setFlags(window->flags() & ~Qt::WindowMinimizeButtonHint);
        // 或者添加一个标志
        // window->setFlags(window->flags() | Qt::CustomizeWindowHint);
    }

    return app.exec();
}
