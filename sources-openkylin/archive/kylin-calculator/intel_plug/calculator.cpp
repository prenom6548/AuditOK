#include <QApplication>
#include <QStringList>
#include <QTranslator>
#include <QLocale>
#include <QStandardPaths>
#include <QLibraryInfo>
#include <QDir>
#include <fcntl.h>
#include <syslog.h>
#include <QTranslator>
#include <QLocale>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QDateTime>
#include <sys/inotify.h>

#include "calculator.h"
#include "data_warehouse.h"
#include "src/mainwindow.h"

const QString Calculator::name() const
{
    return "Calculator";
}

const QString Calculator::nameCN() const
{
    return QString("计算器");
}

const QString Calculator::description() const
{
    return QString("计算器");
}

int Calculator::sortNum() const
{
    return 3;
}

QWidget *Calculator::createWidget(QWidget *parent)
{
    /* 设置不跟随系统字号大小改变 */
    qApp->setProperty("noChangeSystemFontSize", true);

    /* 加载翻译文件 */
    QString tranPath("/usr/share/kylin-calculator/translations/");
    QTranslator *tran = new QTranslator;
    if (tran->load(QLocale() , QString("kylin-calculator") , QString("_") , tranPath)) {
        QApplication::installTranslator(tran);
    } else {
        qDebug() << "Waring : load translation file fail";
    }

    /* 注意 : 数据仓库需在界面前进行实例 , 界面实例时会访问数据仓库中的数据 */
    DataWarehouse::getInstance();

    /* 实例主界面 */
    MainWindow::getInstance();

    printf("Info : calculator plug start ...\n");

    return MainWindow::getInstance();
}
