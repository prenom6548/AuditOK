#include <QStringList>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QDir>
#include <QDebug>
#include <QSessionManager>
#include <singleapplication.h>

#include "log.hpp"
#include "windowmanage.hpp"

#include "mainwindow.h"
#include "data_warehouse.h"
#include "logic_center.h"

int main(int argc, char *argv[])
{
    qInstallMessageHandler(kabase::Log::logOutput);
    kabase::WindowManage::setScalingProperties();
    kdk::QtSingleApplication a(argc, argv);
    a.setApplicationVersion("1.2.0.5");
    a.setWindowIcon(QIcon::fromTheme("kylin-calculator"));

    /*
    * fix: 26797 【好用工程】系统自带的计算器无法多开
    */
//    if (a.isRunning()) {
//        qDebug() << "is running";
//        a.sendMessage("running , 4000");
//        return 0;
//    }

	/*
	 * fix: #bug 159686 【会话管理器】设置重启后恢复上一次会话退出时未关闭的应用，计算器应用未恢复
	 */
	QGuiApplication::setFallbackSessionManagementEnabled(false);
	QObject::connect(qApp, &QApplication::commitDataRequest, [](QSessionManager &manager) {
		if (manager.allowsInteraction())
			qDebug() << "all interect";
		else
			qDebug() << "not allow interect yet";
	});

    /* 加载翻译文件 */
#ifdef NOTKAIMING
    QString tranPath("/usr/share/kylin-calculator/translations/");
#else
    QString tranPath("/opt/apps/top.openkylin.kylin-calculator/files");
#endif
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

    // 加载sdk控件翻译
    QTranslator trans;
    QString locale = QLocale::system().name();
    if(trans.load(":/translations/gui_" + locale  + ".qm")){
        a.installTranslator(&trans);
    }

    /* 实例 */
    LogicCenter::getInstance();

    /* 添加窗管协议 */
    kabase::WindowManage::removeHeader(MainWindow::getInstance());
    a.setActivationWindow(MainWindow::getInstance());
    a.setApplicationName(QObject::tr("Calculator"));
    MainWindow::getInstance()->show();

    return a.exec();
}
