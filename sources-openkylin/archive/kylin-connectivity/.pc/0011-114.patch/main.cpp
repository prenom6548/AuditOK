#include <QApplication>
#include <QLibraryInfo>
#include <QTranslator>
#include <QMessageBox>
#include <QScreen>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QLocale>
#include <fcntl.h>
#include <syslog.h>
#include <singleapplication.h>
#include "log.hpp"
#include "windowmanage.hpp"

#include <ui/mainwindow.h>
#include "dbus/dbusvfsinterface.h"
#include "log.hpp"
#include "windowmanage.hpp"

const QString VERSION = "1.1.0.1";

bool initVLC()
{
    QFileInfo file("/bin/vlc");
    return file.isExecutable();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(kabase::Log::logOutput);
    setbuf(stdout, NULL);
    // set env
#ifdef Q_OS_LINUX
    qputenv("ADB_PATH", "/usr/lib/android-sdk/platform-tools/adb");
    qputenv("SERVER_PATH", "/usr/share/kylin-connectivity/scrcpy-server.jar");
    qputenv("CONFIG_PATH", "/usr/share/kylin-connectivity/config");
    qputenv("KEYMAP_PATH", "/usr/share/kylin-connectivity/keymap");
#endif

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    kabase::WindowManage::setScalingProperties();
    kdk::QtSingleApplication app(argc, argv);

    // update version
    app.setApplicationVersion(VERSION);
    app.setWindowIcon(QIcon::fromTheme("kylin-connectivity"));

    if (app.isRunning()) {
        qWarning() << "kylin-connectivity is already running!";
        app.sendMessage("running , 4000");
        return 0;
    }

    // 添加单例判断
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int singleInstanceFd =
        open(QString(homePath.at(0) + "/.config/kylin-connecitvity%1.lock").arg(getenv("DISPLAY")).toUtf8().data(),
             O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (singleInstanceFd < 0) {
        exit(1);
    }
    if (lockf(singleInstanceFd, F_TLOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, kylin-connectivity is already running!");
        exit(0);
    }

    // 加载sdk控件翻译
    QTranslator trans;
    QString locale = QLocale::system().name();
    if (trans.load(QString(":/translations/gui_%1.qm").arg(locale))) {
        app.installTranslator(&trans);
    } else {
        qWarning() << "Waring: load sdk translation file fail!";
    }

    // 加载翻译
    // 加载自定义翻译文件
    QString tranPath("/usr/share/kylin-connectivity/translations/");
    QTranslator tran;
    if (tran.load(QLocale(), QString("kylin-connectivity"), QString("_"), tranPath)) {
        app.installTranslator(&tran);
    } else {
        qWarning() << "Waring : load translation file fail";
    }

    // 加载qt翻译文件， 更改fileDialog英文
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QTranslator tranQt;
    if (tranQt.load(QLocale(), QString("qt"), QString("_"), qtTransPath)) {
        app.installTranslator(&tranQt);
    } else {
        qWarning() << "Waring : load qt translation file fail";
    }

    MainWindow w;
    w.setSingleFileFd(singleInstanceFd);
    kabase::WindowManage::removeHeader(&w);
    app.setActivationWindow(&w);
    w.show();
    MainWindow *wind = &w;
    kabase::WindowManage::setMiddleOfScreen(&w);
    /* wayland 下最小化拉起 */
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        QObject::connect(&app, &kdk::QtSingleApplication::messageReceived,
                         [wind]() { kabase::WindowManage::activateWindow(wind->getWinId()); });
    }

    if (!initVLC()) {
        qCritical() << "vlc not installed!";
        QMessageBox msgBox;
        msgBox.setText(QObject::tr(
            "In order to ensure the normal operation of the program, please install the VLC program first!"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        w.close();
        exit(1);
    }

    DbusVfsInterface vfsDbus(&w);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService(KYLIN_CONNECTIVITY_SERVICE);
    connection.registerObject(KYLIN_CONNECTIVITY_PATH, &vfsDbus,
                              QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    int ret = app.exec();
    return ret;
}
