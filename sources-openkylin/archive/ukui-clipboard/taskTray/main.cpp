#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "qtsingleapplication.h"
#include <ukui-log4qt.h>
#include "clipboardtray.h"

int  main(int argc, char *argv[])
{
    qputenv("QT_WAYLAND_USE_DATA_CONTROL_ONLY","1");
    initUkuiLog4qt("ukui-clipboard-tray");
    qDebug()<<"ukui-clipboard-tray start!";

    //跟随系统缩放
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //设置该标签会在缩放后，图标异常放大
    //QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QtSingleApplication app(argc,argv);

    QCommandLineParser parser;
    QCommandLineOption showWorkSpaceOption("showMainWindow","show main window");
    parser.addOption(showWorkSpaceOption);
    parser.process(app);

    if (app.isRunning())
    {
        qDebug()<<"ukui-clipboard-tray is running";
        if (parser.isSet("showMainWindow")) {
            app.sendMessage("showMainWindow");
        }
        return EXIT_SUCCESS;
    }

    QTranslator translator;
    const QString locale = QLocale::system().name();
    const QString path = "/usr/share/ukui/translations/ukui-clipboard/ukui-clipboard_" + locale;
    if (translator.load(path)) {
        qDebug()<<"translator path = " << path;
        app.installTranslator(&translator);
    } else {
        qWarning()<< "Load translation file failed";
    }

    //此判断需要放在application初始化后，不然会报莫名错误
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning()<<">>>>>>  系统不支持托盘~";
    }

    ClipboardTray tray;
    if (locale == "ug" || locale == "kk" || locale == "ky") {
        tray.setIsMirror(true);
    }
    QQmlApplicationEngine engine;

    QObject::connect(&app, &QtSingleApplication::messageReceived, [&tray](const QString &message){
        if (message == "showMainWindow") {
            tray.setWindowVisible(true);
            tray.mSystemTray->setVisible(true);
        }
    });

    // 将C++对象注册到QML上下文中
    engine.rootContext()->setContextProperty("tray", &tray);

    // 加载QML文件
    QObject::connect(&tray, &ClipboardTray::firstShowWindow, [&engine, &tray]{
        engine.load(QUrl(QStringLiteral("qrc:/source/ClipboardMainWindow.qml")));
        QWindowList m_windowlist = qGuiApp->allWindows();
        foreach (QWindow *window, m_windowlist) {
            if (window->title() == "Ukui-clipboard MainWindow") {
                tray.getQmlWindow(window);
            }
        }
    });

    return app.exec();
}
