#include "lib/ky-burn-engine/include/burnjobmanager.h"
#include <QtTest/QtTest>


using namespace ky_plugin_burn;
using namespace DFMBURN;
class BurnJobManagerTest : public QObject
{
    Q_OBJECT

private slots:
    // 测试BurnJobManager类的构造函数
    void testJobManagerBurndata() {
        //burn option
        BurnOptions opts;
        BurnJobManager::Config config;
        opts |= BurnOption::kJolietSupport;
        opts |= BurnOption::kKeepAppendable;

        config.opts = opts;
        config.volName = QString("test");

        QString destPath =  QCoreApplication::applicationDirPath().append(QString("/../../test_data"));

        BurnJobManager::instance()->disconnect();
        QEventLoop loop;
        connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
        //start burn job
        BurnJobManager::instance()->startBurnISOFiles(QString("/dev/sr0"), QUrl::fromLocalFile(destPath), config);
        loop.exec();
        QVERIFY(true);
    }

    // 测试BurnJobManager::Config结构体
    void testJobManagerBurnImage() {

        //burn option
        BurnOptions opts;
        BurnJobManager::Config config;
        opts |= BurnOption::kJolietSupport;
        opts |= BurnOption::kKeepAppendable;

        config.opts = opts;
        config.volName = QString("test");

        QString destPath =  QCoreApplication::applicationDirPath().append(QString("/../../test_data"));


        BurnJobManager::instance()->disconnect();
        QEventLoop loop;
        connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
        //start burn job
        BurnJobManager::instance()->startBurnISOImage(QString("/dev/sr0"), QUrl::fromLocalFile(destPath), config);
        loop.exec();
        QVERIFY(true);
        
    }

    // 测试BurnJobManager::Config结构体
    void testJobManagerDumpaImage() {
        QString destPath =  QCoreApplication::applicationDirPath().append(QString("/../../test_data"));

        BurnJobManager::instance()->disconnect();
        QEventLoop loop;
        connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
        //start burn job
        BurnJobManager::instance()->startDumpISOImage(QString("/dev/sr0"), QUrl::fromLocalFile(destPath));
        QVERIFY(true);

    }
    void testJobManagerEraseDisc(){
        BurnJobManager::instance()->disconnect();
        QEventLoop loop;
        connect(BurnJobManager::instance(), &BurnJobManager::currentJobFinished, &loop, &QEventLoop::quit);
        //start burn job
        BurnJobManager::instance()->startEraseDisc(QString("/dev/sr0"));
        QVERIFY(true);
    }
};

QTEST_MAIN(BurnJobManagerTest)
#include "burnjobmanager_test.moc"
