#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QWindow>
#include <QDebug>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <QDir>
#include <fcntl.h>
#include <QDebug>
#include <QSettings>
#include <QQmlContext>
#include <QTranslator>
#include <QQuickWindow>
#include <QtCore>
#include <QObject>
#include <QScreen>
#include <unistd.h>
#include <ukui-log4qt.h>
#include <QWidget>
#include "qtsingleapplication.h"
#include "processmanager.h"
#include "xatom-helper.h"
#include "windowmanager/windowmanager.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "../src/common-tool/comm_func.h"

#define PROC_CPUINFO "/proc/cpuinfo"

void qtVersionHandle()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void qtScaleHandle()
{
    Display *display = XOpenDisplay(nullptr);
    int dpi = QString::fromLatin1(XGetDefault(display, "Xft", "dpi")).toInt();
    XCloseDisplay(display);
    if (dpi <= 0) {
        dpi = 96;
    }
    double scale =  (double) dpi / 96.0;
    qputenv("QT_SCALE_FACTOR", QVariant(scale).toByteArray());
}


// 单例文件锁
bool fileLock()
{
    uid_t user;
    user = geteuid();
    QString dirPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation)
                      + "/ukui-new-function-introduction/";
    QDir dir(dirPath);
    if(dir.exists()) {
        QString lockPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) +
                           "/ukui-new-function-introduction/ukui-new-function-introduction-"+QString::number(user)+".lock";
        umask(S_IRWXO);
        int fd = open(lockPath.toUtf8().data(), O_RDWR | O_CREAT | O_TRUNC,0600);
        if (fd < 0) {
            qDebug()<<"单例文件锁打开异常";
            qDebug()<<"1";
            exit(0);
        }
        return lockf(fd, F_TLOCK, 0);
    } else {
        dir.mkdir(dirPath);//只创建一级子目录，即必须保证上级目录存在
        chmod(dirPath.toUtf8().constData(), 0700);
        QString lockPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) +
                           "/ukui-new-function-introduction/ukui-new-function-introduction-" + QString::number(user)+".lock";
        umask(S_IRWXO);
        int fd = open(lockPath.toUtf8().data(), O_RDWR | O_CREAT | O_TRUNC,0600);
        if (fd < 0) {
            qDebug()<<"单例文件锁打开异常";
            exit(0);
        }
        return lockf(fd, F_TLOCK, 0);
    }
}

// 启动检测配置文件
bool readStartConf()
{
    QString configDirPath = QDir::homePath() + "/.config/ukui";
    QString lockPath = configDirPath + "/ukui-new-function-introduction.conf";

    QDir configDir(configDirPath);
    if (!configDir.exists()) {
        if (!configDir.mkpath(".")) {
            qWarning() << "Failed to create config directory:" << configDirPath;
            return true;
        }
        qWarning() << "Created config directory:" << configDirPath;
    }

    int fd = open(lockPath.toUtf8().data(), O_RDWR | O_TRUNC, 0600);
    if (fd >= 0) {
        qWarning() << "is not first start!!!";
        close(fd);
        return false;
    }

    fd = open(lockPath.toUtf8().data(), O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        qWarning() << "Failed to create config file:" << strerror(errno);
        return true;
    }
    qWarning() << "is first start!!!";
    close(fd);
    return true;
}

// 判断是否为wayland环境
bool isWaylandEnvironment()
{
    if (qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        return false;
    } else {
        return true;
    }
}

bool getIs990()
{
    bool is990 = false;
    QRegExp r1("kirin.*9.0");
    r1.setCaseSensitivity(Qt::CaseInsensitive);
    QRegExp r2("pangu.*m900");
    r2.setCaseSensitivity(Qt::CaseInsensitive);

    QFile file(PROC_CPUINFO);
    if (!file.exists()) {
        is990 = false;
        return is990;
    }
    file.open(QFile::ReadOnly);
    QString str(file.readAll());
    is990 = (str.contains(r1) || str.contains(r2));
    file.close();

    if (is990)
        qDebug() << "is 990";

    return is990;
}

static QPointer<QQuickWindow> g_mainWindow = nullptr;

static void activateQuickWindow()
{
    if (!g_mainWindow) {
        return;
    }

    QQuickWindow *window = g_mainWindow;

    // 确保在主线程执行
    QMetaObject::invokeMethod(qApp, [window]() {
        if (!window) return;

        if (window->visibility() == QWindow::Minimized) {
            window->showNormal();
        }

        window->raise();
        window->requestActivate();

        Qt::WindowFlags flags = window->flags();
        window->setFlags(flags);
        window->show();
        qDebug() << "Window activated:" << window->title();
    }, Qt::QueuedConnection);
}

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-new-function-introduction");
    double scale = 1.0;

    if (getIs990()) {
        // 只有华为机型才需要重新适配缩放
        qputenv("QT_QUICK_BACKEND","software");
        qtScaleHandle();
        scale = QString(qgetenv("QT_SCALE_FACTOR")).toDouble();
        qDebug() << "Scale factor:" << scale;

        QStringList args = {"-platform", "wayland"};
        for (const QString &arg : args) {
            char *argvCstr = strdup(arg.toUtf8().constData());
            argv[argc] = argvCstr;
            argc++;
        }
    }

    QString arg = (argc > 1) ? QString::fromUtf8(argv[1]) : "";
    qtVersionHandle();

    // 检查启动配置
    bool shouldRun = true;
    if (arg != "-nocheck") {
        shouldRun = readStartConf();
        if (!shouldRun) {
            qDebug() << "The demo program has been started. "
                     << "If you want to start it again, "
                     << "please execute [ ukui-new-function-introduction -nocheck] ";
            return 0;
        }
    }

    // 使用 QtSingleApplication
    QtSingleApplication app("ukui-new-function-introduction", argc, argv);

    if (app.isRunning()) {
        qDebug() << "Another instance is already running";
        if (app.sendMessage("activate", 3000)) {
            qDebug() << "Activation message sent successfully";
        } else {
            qDebug() << "Failed to send activation message";
        }
        return 0;
    }



    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QScreen *screen = QGuiApplication::primaryScreen();
    app.setWindowIcon(QIcon::fromTheme("kylin-new-function-introduction"));
    QCoreApplication::setApplicationName("kylin-new-function-introduction");

    qmlRegisterType<processmanager>("org.username", 1, 0, "ProcessManager");
    qRegisterMetaType<processmanager*>("p*");

    QString qm_name = QLocale::system().name();
    QTranslator translator;
    QTranslator trans;

    if(!translator.load("kylin-user-guide_" + qm_name + ".qm",
                       "/usr/share/kylin-user-guide/translations/")) {
        qDebug() << "Failed to load translation file:"
                 << "kylin-user-guide_" + qm_name + ".qm";
    } else {
        app.installTranslator(&translator);
    }

    if(trans.load(":/translations/gui_" + qm_name + ".qm")) {
        app.installTranslator(&trans);
    }

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.load(url);
    engine.rootContext()->setContextProperty("p", new processmanager());

    QObject *rootObject = nullptr;
    if (!engine.rootObjects().isEmpty()) {
        rootObject = engine.rootObjects().at(0);
    }

    QQuickWindow *window = qobject_cast<QQuickWindow *>(rootObject);
    if (window) {
        // 保存全局窗口指针
        g_mainWindow = window;

        window->setTitle(QObject::tr("kylin-new-function-introduction"));
        window->setIcon(QIcon::fromTheme("kylin-new-function-introduction"));
        window->setMinimumSize(QSize(NEWFUNCTION_INTRODUCE_WIDTH, NEWFUNCTION_INTRODUCE_HEIGHT));
        window->setMaximumSize(QSize(NEWFUNCTION_INTRODUCE_WIDTH, NEWFUNCTION_INTRODUCE_HEIGHT));
        kdk::WindowManager::setGeometry(window, QRect(
            ((screen->size().width() - NEWFUNCTION_INTRODUCE_WIDTH) / 2) * scale,
            ((screen->size().height() - NEWFUNCTION_INTRODUCE_HEIGHT) / 2) * scale,
            NEWFUNCTION_INTRODUCE_WIDTH,
            NEWFUNCTION_INTRODUCE_HEIGHT
        ));

        QWidget *dummyWidget = new QWidget();
        dummyWidget->setWindowTitle(window->title());
        dummyWidget->setWindowIcon(window->icon());
        dummyWidget->setGeometry(window->geometry());

        // 设置给 QtSingleApplication
        app.setActivationWindow(dummyWidget, true);
        QObject::connect(&app, &QtSingleApplication::messageReceived,
                         [](const QString &message) {
            qDebug() << "Received message:" << message;
            activateQuickWindow();
        });
        QObject::connect(&app, &QtSingleApplication::activateWindow,
                         []() {
            activateQuickWindow();
        });
        QObject::connect(window, &QObject::destroyed, [dummyWidget]() {
            g_mainWindow.clear();
            if (dummyWidget) {
                dummyWidget->deleteLater();
            }
        });
    }

    return app.exec();
}
