#include <QCoreApplication>
#include <QtTest/QtTest>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDebug>
#include <QIcon>
#include <singleapplication.h>

#include "../kabase/Qt/log.hpp"
#include "../kabase/Qt/windowmanage.hpp"
#include "../src/mainwindow.h"
#include "../logic_center.h"

#include "testclass.h"
#include "testfunlist.h"
#include "testscientificmodel.h"
#include "teststandardmodel.h"
#include "testtoolmodel.h"
#include "testtitlebar.h"
#include "testinputtools.h"
#include "testinputprocess.h"
#include "testconversion.h"
#include "testbigfloat.h"

 
int main(int argc, char *argv[])
{
    TestClass testClass;
    bool isHaWeiKe = false;
    qInstallMessageHandler(kabase::Log::logOutput);
    kabase::WindowManage::setScalingProperties();
    kdk::QtSingleApplication a(argc, argv);
    a.setApplicationVersion("1.2.0.2");
    a.setWindowIcon(QIcon::fromTheme("kylin-calculator"));
    if (a.isRunning()) {
        qDebug() << "is running";
        a.sendMessage("running , 4000");
        return 0;
    }

    /* 加载翻译文件 */
    QString tranPath("/usr/share/kylin-calculator/translations/");
    QString tranQtPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QTranslator *tran = new QTranslator;
    QTranslator *tranQt = new QTranslator;
    if (tran->load(QLocale(), QString("kylin-calculator"), QString("_"), tranPath)) {
        QApplication::installTranslator(tran);
    } else {
        qDebug() << "Waring : load translation file fail";
    }
    if (tranQt->load(QLocale(), QString("qt"), QString("_"), tranQtPath)) {
        QApplication::installTranslator(tranQt);
    } else {
        qDebug() << "Waring : load qt translation file fail";
    }

        QLocale current = QLocale();
    QTranslator *translator = new QTranslator;
    if (translator->load(current, QLatin1String("gui"), QLatin1String("_"),QLatin1String(":translations"))){
        a.installTranslator(translator);
    }
    else{
        qDebug() << "cannot load translator gui_" << QLocale::system().name() <<".qm!";
    }
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ" ){
        isHaWeiKe= true;
    }else{
        isHaWeiKe= false;
    }
    /* 实例 */
    LogicCenter::getInstance();

    QString platform = QGuiApplication::platformName();
//     if(platform.startsWith(QLatin1String("Wayland"),Qt::CaseInsensitive) || QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
//         MainWindow::getInstance()->setIsWayland(true);
//     }


    /* 添加窗管协议 */
    kabase::WindowManage::removeHeader(MainWindow::getInstance());

    /* 移动到窗口中间 */
    // QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    // MainWindow::getInstance()->move(availableGeometry.width() / 2 - MainWindow::getInstance()->width() / 2,
    //                                 availableGeometry.height() / 2 - MainWindow::getInstance()->height() / 2);

    a.setActivationWindow(MainWindow::getInstance());
    if(isHaWeiKe)
        a.setLayoutDirection(Qt::RightToLeft);

    /* wayland 下最小化拉起 */
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        // QObject::connect(&a, &kdk::QtSingleApplication::messageReceived, [=]() {
        //     kabase::WindowManage::activateWindow(MainWindow::getInstance()->getWinId());
        // });
    }

    MainWindow::getInstance()->show();
    if (MainWindow::getInstance()->isTabletMode()) {
        MainWindow::getInstance()->setWindowState(MainWindow::getInstance()->windowState() | Qt::WindowFullScreen);
    }
    else {
        kabase::WindowManage::setMiddleOfScreen(MainWindow::getInstance());
    }

    QTest::qExec(&testClass);
    QTest::qExec(new TestFunList, argc, argv);
    QTest::qExec(new TestScientificModel, argc, argv);
    QTest::qExec(new TestStandardModel, argc, argv);
    QTest::qExec(new TestToolModel, argc, argv);
    QTest::qExec(new TestTitleBar, argc, argv);
    QTest::qExec(new TestInputTools, argc, argv);
    QTest::qExec(new TestInputProcess, argc, argv);
    QTest::qExec(new TestBigFloat, argc, argv);
    QTest::qExec(new TestConversion, argc, argv);

    //return a.exec();
}
